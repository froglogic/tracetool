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

#ifndef TRACE_SERVER_H
#define TRACE_SERVER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QXmlStreamReader>

#include "database.h"
#include "xmlcontenthandler.h"
#include "databasefeeder.h"

class ClientSocket : public QTcpSocket
{
    Q_OBJECT
public:
    ClientSocket( QObject *parent = 0 );

signals:
    void dataReceived( const QByteArray &data );

private slots:
    void handleIncomingData();
};

class NetworkingThread : public QThread
{
    Q_OBJECT
public:
    NetworkingThread( int socketDescriptor, QObject *parent = 0 );

signals:
    void dataReceived( const QByteArray &data );

protected:
    virtual void run();

private:
    int m_socketDescriptor;
    ClientSocket *m_clientSocket;
};

class Server;

class ServerSocket : public QTcpServer
{
public:
    ServerSocket( Server *server );
    ~ServerSocket();

protected:
    virtual void incomingConnection( int socketDescriptor );

private:
    Server *m_server;
    QList<NetworkingThread *> m_networkingThreads;
};

class Server;

class GUIConnection : public QObject
{
    Q_OBJECT
public:
    GUIConnection( Server *server, QTcpSocket *sock );

    void write( const QByteArray &data );

signals:
    void databaseNukeRequested();
    void disconnected( GUIConnection *c );

private slots:
    void handleIncomingData();
    void handleDisconnect();

private:
    Server *m_server;
    QTcpSocket *m_sock;
};

class Server : public QObject, public DatabaseFeeder
{
    Q_OBJECT
public:
    Server( const QString &traceFile,
            QSqlDatabase database, unsigned short port, unsigned short guiPort,
            QObject *parent = 0 );

public slots:
    void handleIncomingData(const QByteArray &data);

signals:
    void traceEntryReceived( const TraceEntry &e );
    void processShutdown( const ProcessShutdownEvent &e );

private slots:
    void handleNewGUIConnection();
    void nukeDatabase();
    void guiDisconnected( GUIConnection *c );

private:
    void handleDatagram( const QByteArray &datagram );
    void handleTraceEntry( const TraceEntry &e );
    void handleShutdownEvent( const ProcessShutdownEvent &ev );
    void archivedEntries();

    QTcpServer *m_guiServer;
    ServerSocket *m_tcpServer;
    XmlContentHandler m_xmlHandler;
    bool m_receivedData;
    QString m_traceFile;
    QList<GUIConnection *> m_guiConnections;
};

#endif // !defined(TRACE_SERVER_H)

