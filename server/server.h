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

struct StorageConfiguration
{
    static const unsigned long UnlimitedTraceSize = 0;

    StorageConfiguration()
        : maximumSize( UnlimitedTraceSize ),
          shrinkBy( 10 )
    { }

    unsigned long maximumSize;
    unsigned short shrinkBy;
    QString archiveDir;
};

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

class XmlContentHandler
{
public:
    XmlContentHandler( Server *server );

    void addData( const QByteArray &data );

    void continueParsing();

private:
    void handleStartElement();
    void handleEndElement();

    QXmlStreamReader m_xmlReader;
    Server *m_server;
    TraceEntry m_currentEntry;
    Variable m_currentVariable;
    QString m_s;
    unsigned long m_currentLineNo;
    StackFrame m_currentFrame;
    bool m_inFrameElement;
    ProcessShutdownEvent m_currentShutdownEvent;
    StorageConfiguration m_currentStorageConfig;
    TraceKey m_currentTraceKey;
};

class Server : public QObject
{
    friend class XmlContentHandler;

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
    void storeEntry( const TraceEntry &e );
    void storeShutdownEvent( const ProcessShutdownEvent &ev );
    void handleDatagram( const QByteArray &datagram );
    void handleTraceEntry( const TraceEntry &e );
    void handleShutdownEvent( const ProcessShutdownEvent &ev );
    void applyStorageConfiguration( const StorageConfiguration &cfg );

    QTcpServer *m_guiServer;
    ServerSocket *m_tcpServer;
    QSqlDatabase m_db;
    XmlContentHandler m_xmlHandler;
    bool m_receivedData;
    QString m_traceFile;
    QList<GUIConnection *> m_guiConnections;
    QString m_archiveDir;
    unsigned short m_shrinkBy;
    unsigned long m_maximumSize;
};

#endif // !defined(TRACE_SERVER_H)

