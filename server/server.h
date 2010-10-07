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
#include <QXmlInputSource>
#include <QXmlSimpleReader>

#include "database.h"

class QSignalMapper;

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

class XmlContentHandler;

class Server : public QObject
{
    friend class XmlContentHandler;

    Q_OBJECT
public:
    Server( const QString &traceFile,
            QSqlDatabase database, unsigned short port,
            QObject *parent = 0 );
    virtual ~Server();

public slots:
    void handleIncomingData(const QByteArray &data);

signals:
    void traceEntryReceived( const TraceEntry &e );
    void processShutdown( const ProcessShutdownEvent &e );

private slots:
    void handleNewGUIConnection();
    void guiSocketDisconnected( QObject *socket );

private:
    void storeEntry( const TraceEntry &e );
    void storeShutdownEvent( const ProcessShutdownEvent &ev );
    void handleDatagram( const QByteArray &datagram );
    void handleTraceEntry( const TraceEntry &e );
    void handleShutdownEvent( const ProcessShutdownEvent &ev );

    QTcpServer *m_guiServer;
    ServerSocket *m_tcpServer;
    QSqlDatabase m_db;
    XmlContentHandler *m_xmlHandler;
    QXmlSimpleReader m_xmlReader;
    QXmlInputSource m_xmlInput;
    bool m_receivedData;
    QString m_traceFile;
    QSignalMapper *m_guiSocketSignalMapper;
    QList<QTcpSocket *> m_guiSockets;
};

#endif // !defined(TRACE_SERVER_H)

