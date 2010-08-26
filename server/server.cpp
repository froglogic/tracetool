#include "server.h"

#include "database.h"

#include <QDomDocument>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVariant>

#include <cassert>

using namespace std;

static TraceEntry deserializeTraceEntry( const QDomElement &e )
{
    TraceEntry entry;
    entry.pid = e.attribute( "pid" ).toUInt();
    entry.tid = e.attribute( "tid" ).toUInt();
    entry.timestamp = QDateTime::fromTime_t( e.attribute( "time" ).toUInt() );
    entry.processName = e.namedItem( "processname" ).toElement().text();
    entry.verbosity = e.namedItem( "verbosity" ).toElement().text().toUInt();
    entry.type = e.namedItem( "type" ).toElement().text().toUInt();
    entry.path = e.namedItem( "location" ).toElement().text();
    entry.lineno = e.namedItem( "location" ).toElement().attribute( "lineno" ).toULong();
    entry.function = e.namedItem( "function" ).toElement().text();
    entry.message = e.namedItem( "message" ).toElement().text();

    QDomElement variablesElement = e.namedItem( "variables" ).toElement();
    if ( !variablesElement.isNull() ) {
        QDomNode n = variablesElement.firstChild();
        while ( !n.isNull() ) {
            QDomElement varElement = n.toElement();

            Variable var;
            var.name = varElement.attribute( "name" );

            const QString typeStr = varElement.attribute( "type" );
            if ( typeStr == "string" ) {
                var.type = Variable::StringType;
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

Server::Server( const QString &databaseFileName, unsigned short port,
                QObject *parent )
    : QObject( parent ),
    m_tcpServer( 0 )
{
    static const char *schemaStatements[] = {
        "CREATE TABLE schema_downgrade (from_version INTEGER,"
                                      " statements TEXT);",
        "INSERT (1, 'SELECT * FROM FOO') INTO schema_downgrade;",
        "CREATE TABLE trace_entry (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                  " traced_thread_id INTEGER,"
                                  " timestamp DATETIME,"
                                  " trace_point_id INTEGER,"
                                  " message TEXT);",
        "CREATE TABLE trace_point (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                  " verbosity INTEGER,"
                                  " type INTEGER,"
                                  " path_id INTEGER,"
                                  " line INTEGER,"
                                  " function_id INTEGER,"
                                  " UNIQUE(verbosity, type, path_id, line, function_id));",
        "CREATE TABLE function_name (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                    " name TEXT,"
                                    " UNIQUE(name));",
        "CREATE TABLE path_name (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                " name TEXT,"
                                " UNIQUE(name));",
        "CREATE TABLE process (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                " name TEXT,"
                                " pid INTEGER,"
	                        " start_time DATETIME,"
	                        " end_time DATETIME,"
                                " UNIQUE(name, pid));",
        "CREATE TABLE traced_thread (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                " process_id INTEGER,"
                                " tid INTEGER,"
                                " UNIQUE(process_id, tid));",
        "CREATE TABLE variable (trace_entry_id INTEGER,"
                               " name TEXT,"
                               " value TEXT,"
                               " type INTEGER);",
        "CREATE TABLE stackframe (trace_entry_id INTEGER,"
                                " depth INTEGER,"
                                " module_name TEXT,"
                                " function_name TEXT,"
                                " offset INTEGER,"
                                " file_name TEXT,"
                                " line INTEGER);"
    };

    const bool initializeDatabase = !QFile::exists( databaseFileName );

    m_db = QSqlDatabase::addDatabase( "QSQLITE", "server" );
    m_db.setDatabaseName( databaseFileName );
    if ( !m_db.open() ) {
        qWarning() << "Failed to open SQL database";
        return;
    }

    if ( initializeDatabase ) {
        QSqlQuery query( m_db );
        query.exec( "BEGIN TRANSACTION;" );
        for ( int i = 0; i < sizeof(schemaStatements) / sizeof(schemaStatements[0]); ++i ) {
            query.exec( schemaStatements[i] );
        }
        query.exec( "COMMIT;" );
    } else {
        QString errMsg;
        if ( !Database::checkCompatibility( m_db, &errMsg ) ) {
            qWarning() << tr("Error on version check: %1").arg( errMsg );
            return;
        }
    }

    m_tcpServer = new QTcpServer( this );
    connect( m_tcpServer, SIGNAL( newConnection() ),
             this, SLOT( handleNewConnection() ) );

    m_tcpServer->listen( QHostAddress::Any, port );
}

void Server::handleNewConnection()
{
    QTcpSocket *client = m_tcpServer->nextPendingConnection();
    connect( client, SIGNAL( readyRead() ), this, SLOT( handleIncomingData() ) );
}

void Server::handleTraceEntryXMLData( const QByteArray &data )
{
    QString errorMsg;
    int errorLine, errorColumn;
    QDomDocument doc;
    if ( !doc.setContent( data, false, &errorMsg, &errorLine, &errorColumn ) ) {
        qWarning() << "Error in incoming XML data: in row" << errorLine << "column" << errorColumn << ":" << errorMsg;
        qWarning() << "Received data:" << data;
        return;
    }

    const TraceEntry e = deserializeTraceEntry( doc.documentElement() );
    storeEntry( e );
    emit traceEntryReceived( e );
}

void Server::handleIncomingData()
{
    QTcpSocket *client = (QTcpSocket *)sender(); // XXX yuck

    const QByteArray xmlData = client->readAll();
    int p = 0;
    int q = xmlData.indexOf( "<traceentry ", p + 1 );
    while ( q != -1 ) {
        handleTraceEntryXMLData( QByteArray::fromRawData( xmlData.data() + p, q - p ) );
        p = q;
        q = xmlData.indexOf( "<traceentry ", p + 1 );
    }
    handleTraceEntryXMLData( QByteArray::fromRawData( xmlData.data() + p, xmlData.size() - p ) );
}

template <typename T>
QString Server::formatValue( const T &v ) const
{
    const QVariant variant = QVariant::fromValue( v );
    QSqlField field( QString(), variant.type() );
    field.setValue( variant );
    return m_db.driver()->formatValue( field );
}

void Server::storeEntry( const TraceEntry &e )
{
    QSqlQuery query( m_db );
    query.setForwardOnly( true );

    query.exec( "BEGIN TRANSACTION;" );

    unsigned int pathId;
    bool ok;
    {
        query.exec( QString( "SELECT id FROM path_name WHERE name=%1;" ).arg( formatValue( e.path ) ) );
        if ( !query.next() ) {
            query.exec( QString( "INSERT INTO path_name VALUES(NULL, %1);" ).arg( formatValue( e.path ) ) );
            query.exec( "SELECT last_insert_rowid() FROM path_name LIMIT 1;" );
            query.next();
        }
        pathId = query.value( 0 ).toUInt( &ok );
        if ( !ok ) {
            qWarning() << "Database corrupt? Got non-numeric integer field";
        }
    }

    unsigned int functionId;
    {
        query.exec( QString( "SELECT id FROM function_name WHERE name=%1;" ).arg( formatValue( e.function ) ) );
        if ( !query.next() ) {
            query.exec( QString( "INSERT INTO function_name VALUES(NULL, %1);" ).arg( formatValue( e.function ) ) );
            query.exec( "SELECT last_insert_rowid() FROM function_name LIMIT 1;" );
            query.next();
        }
        functionId = query.value( 0 ).toUInt( &ok );
        if ( !ok ) {
            qWarning() << "Database corrupt? Got non-numeric integer field";
        }
    }

    unsigned int processId;
    {
        query.exec( QString( "SELECT id FROM process WHERE name=%1 AND pid=%2;" ).arg( formatValue( e.processName ) ).arg( e.pid ) );
        if ( !query.next() ) {
            query.exec( QString( "INSERT INTO process VALUES(NULL, %1, %2);" ).arg( formatValue( e.processName ) ).arg( e.pid ) );
            query.exec( "SELECT last_insert_rowid() FROM process LIMIT 1;" );
            query.next();
        }
        processId = query.value( 0 ).toUInt( &ok );
        if ( !ok ) {
            qWarning() << "Database corrupt? Got non-numeric integer field";
        }
    }

    unsigned int tracedThreadId;
    {
        query.exec( QString( "SELECT id FROM traced_thread WHERE process_id=%1 AND tid=%2;" ).arg( processId ).arg( e.tid ) );
        if ( !query.next() ) {
            query.exec( QString( "INSERT INTO traced_thread VALUES(NULL, %1, %2);" ).arg( processId ).arg( e.tid ) );
            query.exec( "SELECT last_insert_rowid() FROM traced_thread LIMIT 1;" );
            query.next();
        }
        tracedThreadId = query.value( 0 ).toUInt( &ok );
        if ( !ok ) {
            qWarning() << "Database corrupt? Got non-numeric integer field";
        }
    }

    unsigned int tracepointId;
    {
        query.exec( QString( "SELECT id FROM trace_point WHERE verbosity=%1 AND type=%2 AND path_id=%3 AND line=%4 AND function_id=%5;" ).arg( e.verbosity ).arg( e.type ).arg( pathId ).arg( e.lineno ).arg( functionId ) );
        if ( !query.next() ) {
            query.exec( QString( "INSERT INTO trace_point VALUES(NULL, %1, %2, %3, %4, %5);" ).arg( e.verbosity ).arg( e.type ).arg( pathId ).arg( e.lineno ).arg( functionId ) );
            query.exec( "SELECT last_insert_rowid() FROM trace_point LIMIT 1;" );
            query.next();
        }
        tracepointId = query.value( 0 ).toUInt( &ok );
        if ( !ok ) {
            qWarning() << "Database corrupt? Got non-numeric integer field";
        }
    }

    query.exec( QString( "INSERT INTO trace_entry VALUES(NULL, %1, %2, %3, %4)" )
                    .arg( tracedThreadId )
                    .arg( formatValue( e.timestamp ) )
                    .arg( tracepointId )
                    .arg( formatValue( e.message ) ) );
    query.exec( "SELECT last_insert_rowid() FROM trace_entry LIMIT 1;" );
    query.next();
    const unsigned int traceentryId = query.value( 0 ).toUInt();

    {
        QList<Variable>::ConstIterator it, end = e.variables.end();
        for ( it = e.variables.begin(); it != end; ++it ) {
            int typeCode = 0;
            switch ( it->type ) {
                case Variable::StringType:
                    typeCode = 0;
                    break;
                default:
                    assert( !"Unreachable" );
            }

            query.exec( QString( "INSERT INTO variable VALUES(%1, %2, %3, %4);" ).arg( traceentryId ).arg( formatValue( it->name ) ).arg( formatValue( it->value ) ).arg( typeCode ) );
        }
    }

    {
        unsigned int depthCount = 0;
        QList<StackFrame>::ConstIterator it, end = e.backtrace.end();
        for ( it = e.backtrace.begin(); it != end; ++it, ++depthCount ) {
            query.exec( QString( "INSERT INTO stackframe VALUES(%1, %2, %3, %4, %5, %6, %7);" ).arg( traceentryId ).arg( depthCount ).arg( formatValue( it->module ) ).arg( formatValue( it->function ) ).arg( it->functionOffset ).arg( formatValue( it->sourceFile ) ).arg( it->lineNumber ) );
        }
    }

    query.exec( "COMMIT;" );
}

