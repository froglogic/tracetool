/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "server.h"

#include "database.h"
#include "datagramtypes.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>

#include <cassert>
#include <stdexcept>

using namespace std;

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
    connect( m_clientSocket, SIGNAL( disconnected() ),
             this, SLOT( quit() ),
             Qt::QueuedConnection  );
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

GUIConnection::GUIConnection( Server *server, QTcpSocket *sock )
    : QObject( server ),
    m_server( server ),
    m_sock( sock )
{
    connect( m_sock, SIGNAL( readyRead() ), SLOT( handleIncomingData() ) );
    connect( m_sock, SIGNAL( disconnected() ), SLOT( handleDisconnect() ) );
}

void GUIConnection::write( const QByteArray &data )
{
    m_sock->write( data );
}

// Mostly duplicated in gui/mainwindow.cpp (ServerSocket::handleIncomingData)
void GUIConnection::handleIncomingData()
{
    QDataStream stream(m_sock);
    stream.setVersion(QDataStream::Qt_4_0);

    while (true) {
        static quint16 nextPayloadSize = 0;
        if (nextPayloadSize == 0) {
            if (m_sock->bytesAvailable() < sizeof(nextPayloadSize)) {
                return;
            }
            stream >> nextPayloadSize;
        }

        if (m_sock->bytesAvailable() < nextPayloadSize) {
            return;
        }

        quint32 magicCookie;
        stream >> magicCookie;
        if (magicCookie != MagicServerProtocolCookie) {
            m_sock->disconnectFromHost();
            return;
        }

        quint32 protocolVersion;
        stream >> protocolVersion;
        assert(protocolVersion == 1);

        quint8 datagramType;
        stream >> datagramType;
        switch (static_cast<ServerDatagramType>(datagramType)) {
            case DatabaseNukeDatagram:
                emit databaseNukeRequested();
                break;
        }
        nextPayloadSize = 0;
    }
}

void GUIConnection::handleDisconnect()
{
    emit disconnected( this );
    m_sock->deleteLater();
    delete this;
}

Server::Server( const QString &traceFile,
                QSqlDatabase database,
                unsigned short port, unsigned short guiPort,
                QObject *parent )
    : QObject( parent ),
      DatabaseFeeder( database ),
      m_tcpServer( 0 ),
      m_xmlHandler( this )
{
    QFileInfo fi( traceFile );
    m_traceFile = QDir::toNativeSeparators( fi.canonicalFilePath() );

    m_tcpServer = new ServerSocket( this );
    m_tcpServer->listen( QHostAddress::Any, port );

    m_guiServer = new QTcpServer( this );
    connect( m_guiServer, SIGNAL( newConnection() ), SLOT( handleNewGUIConnection() ) );
    m_guiServer->listen( QHostAddress::LocalHost, guiPort );

    m_xmlHandler.addData( "<toplevel_trace_element>" );
}

// duplicated in gui/mainwindow.cpp
template <typename DatagramType, typename ValueType>
QByteArray serializeDatagram( DatagramType type, const ValueType *v )
{
    QByteArray payload;
    {
        static const quint32 ProtocolVersion = 1;

        QDataStream stream( &payload, QIODevice::WriteOnly );
        stream.setVersion( QDataStream::Qt_4_0 );
        stream << MagicServerProtocolCookie << ProtocolVersion << (quint8)type;
        if ( v ) {
            stream << *v;
        }
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

QByteArray serializeGUIClientData( ServerDatagramType type ) {
    return serializeDatagram( type, (int *)0 );
}

template <typename T>
QByteArray serializeGUIClientData( ServerDatagramType type, const T &v ) {
    return serializeDatagram( type, &v );
}

void Server::handleTraceEntry( const TraceEntry &entry )
{
    DatabaseFeeder::handleTraceEntry( entry );

    QByteArray serializedEntry = serializeGUIClientData( TraceEntryDatagram, entry );

    QList<GUIConnection *>::Iterator it, end = m_guiConnections.end();
    for ( it = m_guiConnections.begin(); it != end; ++it ) {
        ( *it )->write( serializedEntry );
    }

    emit traceEntryReceived( entry );
}

void Server::handleShutdownEvent( const ProcessShutdownEvent &ev )
{
    DatabaseFeeder::handleShutdownEvent( ev );

    QByteArray serializedEvent = serializeGUIClientData( ProcessShutdownEventDatagram, ev );

    QList<GUIConnection *>::Iterator it, end = m_guiConnections.end();
    for ( it = m_guiConnections.begin(); it != end; ++it ) {
        ( *it )->write( serializedEvent );
    }

    emit processShutdown( ev );
}

void Server::handleIncomingData( const QByteArray &xmlData )
{
    try {
        m_xmlHandler.addData( xmlData );
        m_xmlHandler.continueParsing();
    } catch ( const runtime_error &e ) {
        qWarning() << e.what();
    }
}

void Server::archivedEntries()
{
    QByteArray serializedEntry = serializeGUIClientData( DatabaseNukeFinishedDatagram );

    QList<GUIConnection *>::Iterator it, end = m_guiConnections.end();
    for ( it = m_guiConnections.begin(); it != end; ++it ) {
        ( *it )->write( serializedEntry );
    }
}

void Server::handleNewGUIConnection()
{
    GUIConnection *c = new GUIConnection( this, m_guiServer->nextPendingConnection() );
    connect( c, SIGNAL( databaseNukeRequested() ), SLOT( nukeDatabase() ) );
    connect( c, SIGNAL( disconnected( GUIConnection * ) ),
             SLOT( guiDisconnected( GUIConnection * ) ) );
    m_guiConnections.append( c );
    c->write( serializeGUIClientData( TraceFileNameDatagram, m_traceFile ) );
}

void Server::guiDisconnected( GUIConnection *c )
{
    m_guiConnections.removeAll( c );
}

void Server::nukeDatabase()
{
    trimDb();

    QByteArray serializedEntry = serializeGUIClientData( DatabaseNukeFinishedDatagram );

    QList<GUIConnection *>::Iterator it, end = m_guiConnections.end();
    for ( it = m_guiConnections.begin(); it != end; ++it ) {
        ( *it )->write( serializedEntry );
    }
}

