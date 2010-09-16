/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "server.h"

#include "database.h"

#include <QDomDocument>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QVariant>

#include <cassert>
#include <stdexcept>

using namespace std;

static TraceEntry deserializeTraceEntry( const QDomElement &e )
{
    TraceEntry entry;
    entry.pid = e.attribute( "pid" ).toUInt();
    entry.processStartTime = QDateTime::fromTime_t( e.attribute( "process_starttime" ).toUInt() );
    entry.tid = e.attribute( "tid" ).toUInt();
    entry.timestamp = QDateTime::fromTime_t( e.attribute( "time" ).toUInt() );
    entry.processName = e.namedItem( "processname" ).toElement().text();
    entry.stackPosition = e.namedItem( "stackposition" ).toElement().text().toULong();
    entry.verbosity = e.namedItem( "verbosity" ).toElement().text().toUInt();
    entry.type = e.namedItem( "type" ).toElement().text().toUInt();
    entry.path = e.namedItem( "location" ).toElement().text();
    entry.lineno = e.namedItem( "location" ).toElement().attribute( "lineno" ).toULong();
    entry.groupName = e.namedItem( "group" ).toElement().text();
    entry.function = e.namedItem( "function" ).toElement().text();
    entry.message = e.namedItem( "message" ).toElement().text();

    QDomElement variablesElement = e.namedItem( "variables" ).toElement();
    if ( !variablesElement.isNull() ) {
        QDomNode n = variablesElement.firstChild();
        while ( !n.isNull() ) {
            QDomElement varElement = n.toElement();

            Variable var;
            var.name = varElement.attribute( "name" );

            using TRACELIB_NAMESPACE_IDENT(VariableType);
            const QString typeStr = varElement.attribute( "type" );
            if ( typeStr == "string" ) {
                var.type = VariableType::String;
            } else if ( typeStr == "number" ) {
                var.type = VariableType::Number;
            } else if ( typeStr == "float" ) {
                var.type = VariableType::Float;
            } else if ( typeStr == "boolean" ) {
                var.type = VariableType::Boolean;
            }
            var.value = varElement.text();

            entry.variables.append( var );

            n = n.nextSibling();
        }
    }

    QDomElement backtraceElement = e.namedItem( "backtrace" ).toElement();
    if ( !backtraceElement.isNull() ) {
        QDomNode n = backtraceElement.firstChild();
        while ( !n.isNull() ) {
            QDomElement frameElement = n.toElement();

            StackFrame frame;
            frame.module = frameElement.namedItem( "module" ).toElement().text();

            QDomElement functionElement = frameElement.namedItem( "function" ).toElement();
            frame.function = functionElement.text();
            frame.functionOffset = functionElement.attribute( "offset" ).toUInt();

            QDomElement locationElement = frameElement.namedItem( "location" ).toElement();
            frame.sourceFile = locationElement.text();
            frame.lineNumber = locationElement.attribute( "lineno" ).toUInt();

            entry.backtrace.append( frame );

            n = n.nextSibling();
        }
    }

    return entry;
}

static ProcessShutdownEvent deserializeShutdownEvent( const QDomElement &e )
{
    ProcessShutdownEvent ev;
    ev.pid = e.attribute( "pid" ).toUInt();
    ev.startTime = QDateTime::fromTime_t( e.attribute( "starttime" ).toUInt() );
    ev.stopTime = QDateTime::fromTime_t( e.attribute( "endtime" ).toUInt() );
    ev.name = e.text();
    return ev;
}

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

Server::Server( const QString &databaseFileName, unsigned short port,
                QObject *parent )
    : QObject( parent ),
    m_tcpServer( 0 )
{
    QString errMsg;
    if (QFile::exists(databaseFileName)) {
        m_db = Database::open(databaseFileName, &errMsg);
    } else {
        m_db = Database::create(databaseFileName, &errMsg);
    }
    if (!m_db.isValid()) {
        qWarning() << "Failed to open SQL database: " + errMsg;
        return;
    }
    m_db.exec( "PRAGMA synchronous=OFF;");

    m_tcpServer = new ServerSocket( this );
    m_tcpServer->listen( QHostAddress::Any, port );
}

Server::~Server()
{
}

void Server::handleTraceEntryXMLData( const QDomDocument &doc )
{
    const TraceEntry entry = deserializeTraceEntry( doc.documentElement() );
    storeEntry( entry );
    emit traceEntryReceived( entry );
}

void Server::handleShutdownXMLData( const QDomDocument &doc )
{
    const ProcessShutdownEvent ev = deserializeShutdownEvent( doc.documentElement() );
    storeShutdownEvent( ev );
    emit processShutdown( ev );
}

static int nextDatagramStart( const QByteArray &data, int pos )
{
    int p = data.indexOf( '<', pos );
    while ( p != -1 ) {
        if ( data.size() - p >= int(sizeof( "<traceentry " ) - 1) &&
             strncmp( data.data() + p, "<traceentry ", sizeof( "<traceentry " ) - 1 ) == 0 ) {
            return p;
        }
        if ( data.size() - p >= int(sizeof( "<shutdownevent " ) - 1) &&
             strncmp( data.data() + p, "<shutdownevent ", sizeof( "<shutdownevent " ) - 1 ) == 0 ) {
            return p;
        }
        p = data.indexOf( '<', p + 1 );
    }
    return p;
}

void Server::handleIncomingData( const QByteArray &xmlData )
{
    int p = 0;
    int q = nextDatagramStart( xmlData, p + 1 );
    while ( q != -1 ) {
        handleDatagram( QByteArray::fromRawData( xmlData.data() + p, q - p ) );
        p = q;
        q = nextDatagramStart( xmlData, p + 1 );
    }
    handleDatagram( QByteArray::fromRawData( xmlData.data() + p, xmlData.size() - p ) );
}

void Server::handleDatagram( const QByteArray &datagram )
{
    QString errorMsg;
    int errorLine, errorColumn;
    QDomDocument doc;
    if ( !doc.setContent( datagram, false, &errorMsg, &errorLine, &errorColumn ) ) {
        qWarning() << "Error in incoming XML data: in row" << errorLine << "column" << errorColumn << ":" << errorMsg;
        qWarning() << "Received datagram:" << datagram;
        return;
    }

    try {
        if ( datagram.startsWith( "<traceentry " ) ) {
            handleTraceEntryXMLData( doc );
        } else if ( datagram.startsWith( "<shutdownevent " ) ) {
            handleShutdownXMLData( doc );
        } else {
            qWarning() << "Server::handleIncomingData: got unknown datagram:" << datagram;
        }
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
        transaction.exec( "DELETE FROM trace_point_group;" );
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

