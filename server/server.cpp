/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "server.h"

#include "database.h"
#include "datagramtypes.h"

#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QSignalMapper>
#include <QSqlDatabase>
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
        } else if ( lName == "key" ) {
            m_currentEntry.traceKeys.append( m_s.trimmed() );
            m_s.clear();
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
                QSqlDatabase database,
                unsigned short port, unsigned short guiPort,
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
    m_guiServer->listen( QHostAddress::LocalHost, guiPort );

    m_xmlHandler = new XmlContentHandler( this );
    m_xmlReader.setContentHandler( m_xmlHandler );
    m_xmlReader.setErrorHandler( m_xmlHandler );

    m_xmlInput.setData( QString::fromUtf8( "<toplevel_trace_element>" ) );
    m_xmlReader.parse( &m_xmlInput, true );
}

template <typename T>
QByteArray serializeGUIClientData( ServerDatagramType type, const T &v )
{
    QByteArray payload;
    {
        static const quint32 ProtocolVersion = 1;

        QDataStream stream( &payload, QIODevice::WriteOnly );
        stream.setVersion( QDataStream::Qt_4_0 );
        stream << MagicServerProtocolCookie << ProtocolVersion << (quint8)type << v;
    }

    QByteArray data;
    {
        QDataStream stream( &data, QIODevice::WriteOnly );
        stream.setVersion( QDataStream::Qt_4_0 );
        stream << (quint16)payload.size();
        data.append( payload );
    }

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

void Server::storeEntry( const TraceEntry &e )
{
    Transaction transaction( m_db );

    unsigned int pathId;
    bool ok;
    {
        QVariant v = transaction.exec( QString( "SELECT id FROM path_name WHERE name=%1;" ).arg( Database::formatValue( m_db, e.path ) ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO path_name VALUES(NULL, %1);" ).arg( Database::formatValue( m_db, e.path ) ) );
            v = transaction.exec( "SELECT last_insert_rowid() FROM path_name LIMIT 1;" );
        }
        pathId = v.toUInt( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to store entry in database: read non-numeric path id from database - corrupt database?" );
        }
    }

    unsigned int functionId;
    {
        QVariant v = transaction.exec( QString( "SELECT id FROM function_name WHERE name=%1;" ).arg( Database::formatValue( m_db, e.function ) ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO function_name VALUES(NULL, %1);" ).arg( Database::formatValue( m_db, e.function ) ) );
            v = transaction.exec( "SELECT last_insert_rowid() FROM function_name LIMIT 1;" );
        }
        functionId = v.toUInt( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to store entry in database: read non-numeric function id from database - corrupt database?" );
        }
    }

    unsigned int processId;
    {
        QVariant v = transaction.exec( QString( "SELECT id FROM process WHERE pid=%1 AND start_time=%2;" ).arg( e.pid ).arg( Database::formatValue( m_db, e.processStartTime ) ) );
        if ( !v.isValid() ) {
            transaction.exec( QString( "INSERT INTO process VALUES(NULL, %1, %2, %3, 0);" ).arg( Database::formatValue( m_db, e.processName ) ).arg( e.pid ).arg( Database::formatValue( m_db, e.processStartTime ) ) );
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
        if ( !getGroupId( &transaction, e.groupName, &groupId ) ) {
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
                    .arg( Database::formatValue( m_db, e.timestamp ) )
                    .arg( tracepointId )
                    .arg( Database::formatValue( m_db, e.message ) )
                    .arg( e.stackPosition ) );
    const unsigned int traceentryId = transaction.exec( "SELECT last_insert_rowid() FROM trace_entry LIMIT 1;" ).toUInt();

    {
        QList<Variable>::ConstIterator it, end = e.variables.end();
        for ( it = e.variables.begin(); it != end; ++it ) {
            transaction.exec( QString( "INSERT INTO variable VALUES(%1, %2, %3, %4);" ).arg( traceentryId ).arg( Database::formatValue( m_db, it->name ) ).arg( Database::formatValue( m_db, it->value ) ).arg( it->type ) );
        }
    }

    {
        unsigned int depthCount = 0;
        QList<StackFrame>::ConstIterator it, end = e.backtrace.end();
        for ( it = e.backtrace.begin(); it != end; ++it, ++depthCount ) {
            transaction.exec( QString( "INSERT INTO stackframe VALUES(%1, %2, %3, %4, %5, %6, %7);" ).arg( traceentryId ).arg( depthCount ).arg( Database::formatValue( m_db, it->module ) ).arg( Database::formatValue( m_db, it->function ) ).arg( it->functionOffset ).arg( Database::formatValue( m_db, it->sourceFile ) ).arg( it->lineNumber ) );
        }
    }

    {
        QList<QString>::ConstIterator it, end = e.traceKeys.end();
        for ( it = e.traceKeys.begin(); it != end; ++it ) {
            getGroupId( &transaction, *it, 0 );
        }
    }
}

void Server::storeShutdownEvent( const ProcessShutdownEvent &ev )
{
    Transaction transaction( m_db );
    transaction.exec( QString( "UPDATE process SET end_time=%1 WHERE pid=%2 AND start_time=%3;" ).arg( Database::formatValue( m_db, ev.stopTime ) ).arg( ev.pid ).arg( Database::formatValue( m_db, ev.startTime ) ) );
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

bool Server::getGroupId( Transaction *transaction, const QString &name, unsigned int *id )
{
    QVariant v = transaction->exec( QString( "SELECT id FROM trace_point_group WHERE name=%1;" ).arg( Database::formatValue( m_db, name ) ) );
    if ( !v.isValid() ) {
        transaction->exec( QString( "INSERT INTO trace_point_group VALUES(NULL, %1);" ).arg( Database::formatValue( m_db, name ) ) );
        if ( id ) {
            v = transaction->exec( "SELECT last_insert_rowid() FROM trace_point_group LIMIT 1;" );
        }
    }

    if ( !id ) {
        return true;
    }

    bool ok;
    *id = v.toUInt( &ok );
    return ok;
}

