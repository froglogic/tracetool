#include "server.h"

#include <QDomDocument>
#include <QDebug>
#include <QSqlDatabase>
#include <QTcpServer>
#include <QTcpSocket>

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

    QDomDocument doc;
    if ( !doc.setContent( client->readAll(), false ) ) {
        qWarning() << "Error in incoming XML data";
        return;
    }

    TraceEntry e = deserializeTraceEntry( doc.documentElement() );
}

