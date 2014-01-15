/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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

