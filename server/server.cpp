/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "server.h"

#include "database.h"

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QSignalMapper>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QVariant>

#include <cassert>
#include <stdexcept>

using namespace std;

class XmlContentHandler : public QXmlDefaultHandler
{
public:
    XmlContentHandler( Server *server ) : m_server( server ), m_inFrameElement( false ) { }

    virtual bool startElement( const QString &ns, const QString &lName, const QString &qName, const QXmlAttributes &atts )
    {
        if ( lName == "traceentry" ) {
            m_currentEntry = TraceEntry();
            m_currentEntry.pid = atts.value( "pid" ).toUInt();
            m_currentEntry.processStartTime = QDateTime::fromTime_t( atts.value( "process_starttime" ).toUInt() );
            m_currentEntry.tid = atts.value( "tid" ).toUInt();
            m_currentEntry.timestamp = QDateTime::fromTime_t( atts.value( "time" ).toUInt() );
        } else if ( lName == "variable" ) {
            m_currentVariable = Variable();
            m_currentVariable.name = atts.value( "name" );
            const QString typeStr = atts.value( "type" );
            if ( typeStr == "string" ) {
                m_currentVariable.type = TRACELIB_NAMESPACE_IDENT(VariableType)::String;
            } else if ( typeStr == "number" ) {
                m_currentVariable.type = TRACELIB_NAMESPACE_IDENT(VariableType)::Number;
            } else if ( typeStr == "float" ) {
                m_currentVariable.type = TRACELIB_NAMESPACE_IDENT(VariableType)::Float;
            } else if ( typeStr == "boolean" ) {
                m_currentVariable.type = TRACELIB_NAMESPACE_IDENT(VariableType)::Boolean;
            }
        } else if ( lName == "location" ) {
            m_currentLineNo = atts.value( "lineno" ).toULong();
        } else if ( lName == "frame" ) {
            m_inFrameElement = true;
            m_currentFrame = StackFrame();
        } else if ( lName == "function" ) {
            m_currentFrame.functionOffset = atts.value( "offset" ).toUInt();
        } else if ( lName == "shutdownevent" ) {
            m_currentShutdownEvent = ProcessShutdownEvent();
            m_currentShutdownEvent.pid = atts.value( "pid" ).toUInt();
            m_currentShutdownEvent.startTime = QDateTime::fromTime_t( atts.value( "starttime" ).toUInt() );
            m_currentShutdownEvent.stopTime = QDateTime::fromTime_t( atts.value( "endtime" ).toUInt() );
        }
        return true;
    }

    virtual bool characters( const QString &ch ) {
        m_s += ch;
        return true;
    }

    virtual bool endElement( const QString &ns, const QString &lName, const QString &qName )
    {
        if ( lName == "traceentry" ) {
            m_server->handleTraceEntry( m_currentEntry );
        } else if ( lName == "variable" ) {
            m_currentVariable.value = m_s.trimmed();
            m_currentEntry.variables.append( m_currentVariable );
            m_s.clear();
        } else if ( lName == "processname" ) {
            m_currentEntry.processName = m_s.trimmed();
            m_s.clear();
        } else if ( lName == "stackposition" ) {
            m_currentEntry.stackPosition = m_s.trimmed().toULong();
            m_s.clear();
        } else if ( lName == "verbosity" ) {
            m_currentEntry.verbosity = m_s.trimmed().toUInt();
            m_s.clear();
        } else if ( lName == "type" ) {
            m_currentEntry.type = m_s.trimmed().toUInt();
            m_s.clear();
        } else if ( lName == "location" ) {
            if ( m_inFrameElement ) {
                m_currentFrame.sourceFile = m_s.trimmed();
                m_currentFrame.lineNumber = m_currentLineNo;
            } else {
                m_currentEntry.path = m_s.trimmed();
                m_currentEntry.lineno = m_currentLineNo;
            }
            m_s.clear();
        } else if ( lName == "group" ) {
            m_currentEntry.groupName = m_s.trimmed();
            m_s.clear();
        } else if ( lName == "function" ) {
            m_currentEntry.function = m_s.trimmed();
            m_s.clear();
        } else if ( lName == "message" ) {
            m_currentEntry.message = m_s.trimmed();
            m_s.clear();
        } else if ( lName == "module" ) {
            m_currentFrame.module = m_s.trimmed();
            m_s.clear();
        } else if ( lName == "function" ) {
            m_currentFrame.function = m_s.trimmed();
            m_s.clear();
        } else if ( lName == "frame" ) {
            m_inFrameElement = false;
            m_currentEntry.backtrace.append( m_currentFrame );
        } else if ( lName == "shutdownevent" ) {
            m_currentShutdownEvent.name = m_s.trimmed();
            m_s.clear();
            m_server->handleShutdownEvent( m_currentShutdownEvent );
        }
        return true;
    }

private:
    XmlContentHandler( const XmlContentHandler &other ); // disabled
    void operator=( const XmlContentHandler &rhs );

    Server *m_server;
    TraceEntry m_currentEntry;
    Variable m_currentVariable;
    QString m_s;
    unsigned long m_currentLineNo;
    StackFrame m_currentFrame;
    bool m_inFrameElement;
    ProcessShutdownEvent m_currentShutdownEvent;
};

ClientSocket::ClientSocket( QObject *parent )
    : QTcpSocket( parent )
{
    connect( this, SIGNAL( readyRead() ),
             this, SLOT( handleIncomingData() ) );
}

void ClientSocket::handleIncomingData()
{
    const QByteArray data = readAll();
    assert( !data.isEmpty() );
    emit dataReceived( data );
}

NetworkingThread::NetworkingThread( int socketDescriptor, QObject *parent )
    : QThread( parent ),
    m_socketDescriptor( socketDescriptor ),
    m_clientSocket( 0 )
{
}

void NetworkingThread::run()
{
    m_clientSocket = new ClientSocket;
    m_clientSocket->setSocketDescriptor( m_socketDescriptor );
    connect( m_clientSocket, SIGNAL( dataReceived( const QByteArray & ) ),
             this, SIGNAL( dataReceived( const QByteArray & ) ),
             Qt::QueuedConnection );
    exec();
    delete m_clientSocket;
}

ServerSocket::ServerSocket( Server *server )
    : QTcpServer( server ),
    m_server( server )
{
}

ServerSocket::~ServerSocket()
{
    QList<NetworkingThread *>::Iterator it, end = m_networkingThreads.end();
    for ( it = m_networkingThreads.begin(); it != end; ++it ) {
        ( *it )->quit();
    }

    for ( it = m_networkingThreads.begin(); it != end; ++it ) {
        ( *it )->wait();
    }
}

void ServerSocket::incomingConnection( int socketDescriptor )
{
    NetworkingThread *thread = new NetworkingThread( socketDescriptor,
                                                     this );
    m_networkingThreads.push_back( thread );
    connect( thread, SIGNAL( dataReceived( const QByteArray & ) ),
             m_server, SLOT( handleIncomingData( const QByteArray & ) ) );
    connect( thread, SIGNAL( finished() ),
             thread, SLOT( deleteLater() ) );
    thread->start();
}

Server::Server( const QString &traceFile,
                QSqlDatabase database, unsigned short port,
                QObject *parent )
    : QObject( parent ),
      m_tcpServer( 0 ),
      m_db( database ),
      m_xmlHandler( 0 ),
      m_guiSocketSignalMapper( 0 )
{
    QFileInfo fi( traceFile );
    m_traceFile = QDir::toNativeSeparators( fi.canonicalFilePath() );

    assert( m_db.isValid() );
    m_db.exec( "PRAGMA synchronous=OFF;");

    m_guiSocketSignalMapper = new QSignalMapper( this );
    connect( m_guiSocketSignalMapper, SIGNAL( mapped( QObject * ) ),
             this, SLOT( guiSocketDisconnected( QObject * ) ) );

    m_tcpServer = new ServerSocket( this );
    m_tcpServer->listen( QHostAddress::Any, port );

    m_guiServer = new QTcpServer( this );
    connect( m_guiServer, SIGNAL( newConnection() ), SLOT( handleNewGUIConnection() ) );
    m_guiServer->listen( QHostAddress::LocalHost, port + 1 );

    m_xmlHandler = new XmlContentHandler( this );
    m_xmlReader.setContentHandler( m_xmlHandler );
    m_xmlReader.setErrorHandler( m_xmlHandler );

    m_xmlInput.setData( QString::fromUtf8( "<toplevel_trace_element>" ) );
    m_xmlReader.parse( &m_xmlInput, true );
}

const quint32 MagicServerProtocolCookie = 0x22021990;

template <typename T>
QByteArray serializeGUIClientData( ServerDatagramType type, const T &v )
{
    static const quint32 ProtocolVersion = 1;

    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream.setVersion( QDataStream::Qt_4_0 );
    stream << MagicServerProtocolCookie << ProtocolVersion << (quint8)type << v;
    return data;
}

Server::~Server()
{
    delete m_xmlHandler;
}

void Server::handleTraceEntry( const TraceEntry &entry )
{
    storeEntry( entry );

    QByteArray serializedEntry = serializeGUIClientData( TraceEntryDatagram, entry );

    QList<QTcpSocket *>::Iterator it, end = m_guiSockets.end();
    for ( it = m_guiSockets.begin(); it != end; ++it ) {
        ( *it )->write( serializedEntry );
    }

    emit traceEntryReceived( entry );
}

void Server::handleShutdownEvent( const ProcessShutdownEvent &ev )
{
    storeShutdownEvent( ev );

    QByteArray serializedEvent = serializeGUIClientData( ProcessShutdownEventDatagram, ev );

    QList<QTcpSocket *>::Iterator it, end = m_guiSockets.end();
    for ( it = m_guiSockets.begin(); it != end; ++it ) {
        ( *it )->write( serializedEvent );
    }

    emit processShutdown( ev );
}

void Server::handleIncomingData( const QByteArray &xmlData )
{
    try {
        m_xmlInput.setData( xmlData );
        m_xmlReader.parseContinue();
    } catch ( const runtime_error &e ) {
        qWarning() << e.what();
    }
}

template <typename T>
QString Server::formatValue( const T &v ) const
{
    const QVariant variant = QVariant::fromValue( v );
    QSqlField field( QString(), variant.type() );
    field.setValue( variant );
    return m_db.driver()->formatValue( field );
}

class Transaction
{
public:
    Transaction( QSqlDatabase db )
        : m_query( db ), m_commitChanges( true ) {
        m_query.setForwardOnly( true );
        m_query.exec( "BEGIN TRANSACTION;" );
    }

    ~Transaction() {
        m_query.exec( m_commitChanges ? "COMMIT;" : "ROLLBACK;" );
    }

    QVariant exec( const QString &statement ) {
        if ( !m_query.exec( statement ) ) {
            m_commitChanges = false;

            const QString msg = QString( "Failed to store entry in database: executing SQL command '%1' failed: %2" )
                            .arg( statement )
                            .arg( m_query.lastError().text() );
            throw runtime_error( msg.toLatin1().data() );
        }
        if ( m_query.next() ) {
            return m_query.value( 0 );
        }
        return QVariant();
    }

private:
    Transaction( const Transaction &other );
    void operator=( const Transaction &rhs );

    QSqlQuery m_query;
    bool m_commitChanges;
};

void Server::storeEntry( const TraceEntry &e )
{
    Transaction transaction( m_db );

    unsigned int pathId;
    bool ok;
    {
        QVariant v = transaction.exec( QString( "SELECT id FROM path_name WHERE name=%1;" ).arg( formatValue( e.path ) ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO path_name VALUES(NULL, %1);" ).arg( formatValue( e.path ) ) );
            v = transaction.exec( "SELECT last_insert_rowid() FROM path_name LIMIT 1;" );
        }
        pathId = v.toUInt( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to store entry in database: read non-numeric path id from database - corrupt database?" );
        }
    }

    unsigned int functionId;
    {
        QVariant v = transaction.exec( QString( "SELECT id FROM function_name WHERE name=%1;" ).arg( formatValue( e.function ) ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO function_name VALUES(NULL, %1);" ).arg( formatValue( e.function ) ) );
            v = transaction.exec( "SELECT last_insert_rowid() FROM function_name LIMIT 1;" );
        }
        functionId = v.toUInt( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to store entry in database: read non-numeric function id from database - corrupt database?" );
        }
    }

    unsigned int processId;
    {
        QVariant v = transaction.exec( QString( "SELECT id FROM process WHERE pid=%1 AND start_time=%2;" ).arg( e.pid ).arg( formatValue( e.processStartTime ) ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO process VALUES(NULL, %1, %2, %3, 0);" ).arg( formatValue( e.processName ) ).arg( e.pid ).arg( formatValue( e.processStartTime ) ) );
            v = transaction.exec( "SELECT last_insert_rowid() FROM process LIMIT 1;" );
        }
        processId = v.toUInt( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to store entry in database: read non-numeric process id from database - corrupt database?" );
        }
    }

    unsigned int tracedThreadId;
    {
        QVariant v = transaction.exec( QString( "SELECT id FROM traced_thread WHERE process_id=%1 AND tid=%2;" ).arg( processId ).arg( e.tid ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO traced_thread VALUES(NULL, %1, %2);" ).arg( processId ).arg( e.tid ) );
            v = transaction.exec( "SELECT last_insert_rowid() FROM traced_thread LIMIT 1;" );
        }
        tracedThreadId = v.toUInt( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to store entry in database: read non-numeric traced thread id from database - corrupt database?" );
        }
    }

    unsigned int groupId = 0;
    if ( !e.groupName.isNull() ) {
        QVariant v = transaction.exec( QString( "SELECT id FROM trace_point_group WHERE name=%1;" ).arg( formatValue( e.groupName ) ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO trace_point_group VALUES(NULL, %1);" ).arg( formatValue( e.groupName ) ) );
            v = transaction.exec( "SELECT last_insert_rowid() FROM trace_point_group LIMIT 1;" );
        }
        groupId = v.toUInt( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to store entry in database: read non-numeric trace point group id from database - corrupt database?" );
        }
    }

    unsigned int tracepointId;
    {
        QVariant v = transaction.exec( QString( "SELECT id FROM trace_point WHERE verbosity=%1 AND type=%2 AND path_id=%3 AND line=%4 AND function_id=%5 AND group_id=%6;" ).arg( e.verbosity ).arg( e.type ).arg( pathId ).arg( e.lineno ).arg( functionId ).arg( groupId ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO trace_point VALUES(NULL, %1, %2, %3, %4, %5, %6);" ).arg( e.verbosity ).arg( e.type ).arg( pathId ).arg( e.lineno ).arg( functionId ).arg( groupId ) );
            v = transaction.exec( "SELECT last_insert_rowid() FROM trace_point LIMIT 1;" );
        }
        tracepointId = v.toUInt( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to store entry in database: read non-numeric tracepoint id from database - corrupt database?" );
        }
    }

    transaction.exec( QString( "INSERT INTO trace_entry VALUES(NULL, %1, %2, %3, %4, %5)" )
                    .arg( tracedThreadId )
                    .arg( formatValue( e.timestamp ) )
                    .arg( tracepointId )
                    .arg( formatValue( e.message ) )
                    .arg( e.stackPosition ) );
    const unsigned int traceentryId = transaction.exec( "SELECT last_insert_rowid() FROM trace_entry LIMIT 1;" ).toUInt();

    {
        QList<Variable>::ConstIterator it, end = e.variables.end();
        for ( it = e.variables.begin(); it != end; ++it ) {
            transaction.exec( QString( "INSERT INTO variable VALUES(%1, %2, %3, %4);" ).arg( traceentryId ).arg( formatValue( it->name ) ).arg( formatValue( it->value ) ).arg( it->type ) );
        }
    }

    {
        unsigned int depthCount = 0;
        QList<StackFrame>::ConstIterator it, end = e.backtrace.end();
        for ( it = e.backtrace.begin(); it != end; ++it, ++depthCount ) {
            transaction.exec( QString( "INSERT INTO stackframe VALUES(%1, %2, %3, %4, %5, %6, %7);" ).arg( traceentryId ).arg( depthCount ).arg( formatValue( it->module ) ).arg( formatValue( it->function ) ).arg( it->functionOffset ).arg( formatValue( it->sourceFile ) ).arg( it->lineNumber ) );
        }
    }
}

void Server::storeShutdownEvent( const ProcessShutdownEvent &ev )
{
    Transaction transaction( m_db );
    transaction.exec( QString( "UPDATE process SET end_time=%1 WHERE pid=%2 AND start_time=%3;" ).arg( formatValue( ev.stopTime ) ).arg( ev.pid ).arg( formatValue( ev.startTime ) ) );
}

void Server::trimTo( size_t nMostRecent )
{
    /* Special handling in case we want to remove all entries from
     * the database; these simple DELETE FROM statements are
     * recognized by sqlite and they run much faster than those
     * with a WHERE clause.
     */
    if ( nMostRecent == 0 ) {
        Transaction transaction( m_db );
        transaction.exec( "DELETE FROM trace_entry;" );
        transaction.exec( "DELETE FROM trace_point;" );
        transaction.exec( "DELETE FROM function_name;" );
        transaction.exec( "DELETE FROM path_name;" );
        transaction.exec( "DELETE FROM process;" );
        transaction.exec( "DELETE FROM traced_thread;" );
        transaction.exec( "DELETE FROM variable;" );
        transaction.exec( "DELETE FROM stackframe;" );
#if 0 // cache for the user's convenenience
        transaction.exec( "DELETE FROM trace_point_group;" );
#endif
        return;
    }
    qWarning() << "Server::trimTo: deleting all but the n most recent trace "
                  "entries not implemented yet!";
}

QList<StackFrame> Server::backtraceForEntry( unsigned int id )
{
    const QString statement = QString(
                      "SELECT"
                      " module_name,"
                      " function_name,"
                      " offset,"
                      " file_name,"
                      " line "
                      "FROM"
                      " stackframe "
                      "WHERE"
                      " trace_entry_id=%1 "
                      "ORDER BY"
                      " depth" ).arg( id );

    QSqlQuery q( m_db );
    q.setForwardOnly( true );
    if ( !q.exec( statement ) ) {
        const QString msg = QString( "Failed to retrieve backtrace for trace entry: executing SQL command '%1' failed: %2" )
                        .arg( statement )
                        .arg( q.lastError().text() );
        throw runtime_error( msg.toUtf8().constData() );
    }

    QList<StackFrame> frames;
    while ( q.next() ) {
        StackFrame f;
        f.module = q.value( 0 ).toString();
        f.function = q.value( 1 ).toString();
        f.functionOffset = q.value( 2 ).toUInt();
        f.sourceFile = q.value( 3 ).toString();
        f.lineNumber = q.value( 4 ).toUInt();
        frames.append( f );
    }

    return frames;
}

void Server::addGroupId( const QString &id )
{
    Transaction transaction( m_db );
    if ( !transaction.exec( QString( "SELECT id FROM trace_point_group WHERE name=%1;" ).arg( formatValue( id ) ) ).isValid() ) {
        transaction.exec( QString( "INSERT INTO trace_point_group VALUES(NULL, %1);" ).arg( formatValue( id ) ) );
        return;
    }
}

QStringList Server::seenGroupIds() const
{
    const QString statement = QString(
                      "SELECT"
                      " name "
                      "FROM"
                      " trace_point_group;" );

    QSqlQuery q( m_db );
    q.setForwardOnly( true );
    if ( !q.exec( statement ) ) {
        const QString msg = QString( "Failed to retrieve list of available trace groups: executing SQL command '%1' failed: %2" )
                        .arg( statement )
                        .arg( q.lastError().text() );
        throw runtime_error( msg.toUtf8().constData() );
    }

    QStringList l;
    while ( q.next() ) {
        l.append( q.value( 0 ).toString() );
    }
    return l;
}

QList<TracedApplicationInfo> Server::tracedApplications() const
{
    const QString statement = QString(
                      "SELECT"
                      " name,"
                      " pid,"
                      " start_time,"
                      " end_time "
                      "FROM"
                      " process;" );

    QSqlQuery q( m_db );
    q.setForwardOnly( true );
    if ( !q.exec( statement ) ) {
        const QString msg = QString( "Failed to retrieve list of traced applications: executing SQL command '%1' failed: %2" )
                        .arg( statement )
                        .arg( q.lastError().text() );
        throw runtime_error( msg.toUtf8().constData() );
    }

    QList<TracedApplicationInfo> l;
    while ( q.next() ) {
        TracedApplicationInfo info;
        bool ok;
        info.pid = q.value( 1 ).toUInt( &ok );
        assert( ok );
        info.startTime = QDateTime::fromString( q.value( 2 ).toString(), Qt::ISODate );
        info.stopTime = QDateTime::fromString( q.value( 3 ).toString(), Qt::ISODate );
        info.name = q.value( 0 ).toString();

        l.append( info );
    }
    return l;
}

void Server::handleNewGUIConnection()
{
    QTcpSocket *guiSocket = m_guiServer->nextPendingConnection();
    m_guiSockets.append( guiSocket );

    connect( guiSocket, SIGNAL( disconnected() ),
             m_guiSocketSignalMapper, SLOT( map() ) );
    m_guiSocketSignalMapper->setMapping( guiSocket, guiSocket );

    guiSocket->write( serializeGUIClientData( TraceFileNameDatagram, m_traceFile ) );
}

void Server::guiSocketDisconnected( QObject *sock )
{
    QTcpSocket *guiSocket = qobject_cast<QTcpSocket *>( sock );
    assert( guiSocket != 0 );
    m_guiSockets.removeAll( guiSocket );
    guiSocket->deleteLater();
}

QDataStream &operator<<( QDataStream &stream, const TraceEntry &entry )
{
    return stream << (quint32)entry.pid
        << entry.processStartTime
        << entry.processName
        << (quint32)entry.tid
        << entry.timestamp
        << (quint8)entry.verbosity
        << (quint8)entry.type
        << entry.path
        << (quint32)entry.lineno
        << entry.groupName
        << entry.function
        << entry.message
        << entry.variables
        << entry.backtrace
        << (quint64)entry.stackPosition;
}

QDataStream &operator>>( QDataStream &stream, TraceEntry &entry )
{
    quint32 pid, tid, lineno;
    quint8 verbosity, type;
    quint64 stackPosition;

    stream >> pid
        >> entry.processStartTime
        >> entry.processName
        >> tid
        >> entry.timestamp
        >> verbosity
        >> type
        >> entry.path
        >> lineno
        >> entry.groupName
        >> entry.function
        >> entry.message
        >> entry.variables
        >> entry.backtrace
        >> stackPosition;

    entry.pid = pid;
    entry.tid = tid;
    entry.lineno = lineno;
    entry.verbosity = verbosity;
    entry.type = type;
    entry.stackPosition = stackPosition;

    return stream;
}

QDataStream &operator<<( QDataStream &stream, const ProcessShutdownEvent &ev )
{
    return stream << (quint32)ev.pid
        << ev.startTime
        << ev.stopTime
        << ev.name;
}

QDataStream &operator>>( QDataStream &stream, ProcessShutdownEvent &ev )
{
    quint32 pid;
    stream >> pid
        >> ev.startTime
        >> ev.stopTime
        >> ev.name;
    ev.pid = pid;
    return stream;
}

QDataStream &operator<<( QDataStream &stream, const StackFrame &entry )
{
    return stream << entry.module
        << entry.function
        << (quint64)entry.functionOffset
        << entry.sourceFile
        << (quint32)entry.lineNumber;
}

QDataStream &operator>>( QDataStream &stream, StackFrame &entry )
{
    quint64 functionOffset;
    quint32 lineNumber;
    stream >> entry.module
        >> entry.function
        >> functionOffset
        >> entry.sourceFile
        >> lineNumber;
    entry.functionOffset = functionOffset;
    entry.lineNumber = lineNumber;
    return stream;
}

QDataStream &operator<<( QDataStream &stream, const Variable &entry )
{
    return stream << entry.name
        << (quint8)entry.type
        << entry.value;
}

QDataStream &operator>>( QDataStream &stream, Variable &entry )
{
    quint8 type;
    stream >> entry.name
        >> type
        >> entry.value;
    entry.type = (TRACELIB_NAMESPACE_IDENT(VariableType)::Value)type;
    return stream;
}

