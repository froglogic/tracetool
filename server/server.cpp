#include "server.h"

#include <QDomDocument>
#include <QDebug>
#include <QSqlDatabase>
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
    entry.timestamp = e.attribute( "time" ).toULong();
    entry.verbosity = e.namedItem( "verbosity" ).toElement().text().toUInt();
    entry.type = e.namedItem( "type" ).toElement().text().toUInt();
    entry.path = e.namedItem( "location" ).toElement().text();
    entry.lineno = e.namedItem( "location" ).toElement().attribute( "lineno" ).toULong();
    entry.function = e.namedItem( "function" ).toElement().text();
    entry.message = e.namedItem( "message" ).toElement().text();
    return entry;
}

static void storeEntryInDatabase( const TraceEntry &e, QSqlDatabase *db )
{
    QSqlQuery query;
    query.setForwardOnly( true );

    query.exec( "BEGIN TRANSACTION;" );

    unsigned int pathId;
    bool ok;
    {
        query.exec( QString( "SELECT id FROM path_name WHERE name='%1';" ).arg( e.path ) );
        if ( !query.next() ) {
            query.exec( QString( "INSERT INTO path_name VALUES(NULL, '%1');" ).arg( e.path ) );
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
        query.exec( QString( "SELECT id FROM function_name WHERE name='%1';" ).arg( e.function ) );
        if ( !query.next() ) {
            query.exec( QString( "INSERT INTO function_name VALUES(NULL, '%1');" ).arg( e.function ) );
            query.exec( "SELECT last_insert_rowid() FROM function_name LIMIT 1;" );
            query.next();
        }
        functionId = query.value( 0 ).toUInt( &ok );
        if ( !ok ) {
            qWarning() << "Database corrupt? Got non-numeric integer field";
        }
    }

    unsigned int tracepointId;
    {
        query.exec( QString( "SELECT id FROM trace_point WHERE verbosity=%1 AND type=%2 AND path_id=%3 AND line=%4 AND function_id=%5;" ).arg( e.verbosity ).arg( e.type ).arg( pathId ).arg( e.lineno ).arg( functionId ) );
        if ( !query.next() ) {
            query.exec( QString( "INSERT OR IGNORE INTO trace_point VALUES(NULL, %1, %2, %3, %4, %5);" ).arg( e.verbosity ).arg( e.type ).arg( pathId ).arg( e.lineno ).arg( functionId ) );
            query.exec( "SELECT last_insert_rowid() FROM trace_point LIMIT 1;" );
            query.next();
        }
        tracepointId = query.value( 0 ).toUInt( &ok );
        if ( !ok ) {
            qWarning() << "Database corrupt? Got non-numeric integer field";
        }
    }

    query.exec( QString( "INSERT INTO trace_entry VALUES(NULL, %1, %2, %3, %4, '%5');" ).arg( e.pid ).arg( e.tid ).arg( e.timestamp ).arg( tracepointId ).arg( e.message ) );

    query.exec( "COMMIT;" );
}

Server::Server( QObject *parent, QSqlDatabase *db, unsigned short port )
    : QObject( parent ),
    m_tcpServer( 0 ),
    m_db( db )
{
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

void Server::handleIncomingData()
{
    QTcpSocket *client = (QTcpSocket *)sender(); // XXX yuck

    const QByteArray xmlData = client->readAll();

    QDomDocument doc;
    if ( !doc.setContent( xmlData ) ) {
        qWarning() << "Error in incoming XML data";
        return;
    }

    const TraceEntry e = deserializeTraceEntry( doc.documentElement() );
    storeEntryInDatabase( e, m_db );
    emit traceEntryReceived( e );
}

