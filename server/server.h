/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACE_SERVER_H
#define TRACE_SERVER_H

#include <QByteArray>
#include <QDateTime>
#include <QDomDocument>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QTcpServer>
#include <QThread>

#include "../core/tracelib.h"

class QTcpSocket;

struct StackFrame
{
    QString module;
    QString function;
    size_t functionOffset;
    QString sourceFile;
    size_t lineNumber;
};

struct Variable
{
    QString name;
    TRACELIB_NAMESPACE_IDENT(VariableType)::Value type;
    QString value;
};

struct TraceEntry
{
    unsigned int pid;
    QDateTime processStartTime;
    QString processName;
    unsigned int tid;
    QDateTime timestamp;
    unsigned int verbosity;
    unsigned int type;
    QString path;
    unsigned long lineno;
    QString function;
    QString message;
    QList<Variable> variables;
    QList<StackFrame> backtrace;
};

struct ProcessShutdownEvent
{
    unsigned int pid;
    QDateTime startTime;
    QDateTime stopTime;
    QString name;
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

private slots:
    void handleIncomingData();

private:
    int m_socketDescriptor;
    QTcpSocket *m_clientSocket;
};

class Server;

class ServerSocket : public QTcpServer
{
public:
    ServerSocket( Server *server );

protected:
    virtual void incomingConnection( int socketDescriptor );

private:
    Server *m_server;
};

class Server : public QObject
{
    Q_OBJECT
public:
    Server( const QString &databaseFileName, unsigned short port,
            QObject *parent = 0 );
    virtual ~Server();

public slots:
    void handleIncomingData(const QByteArray &data);

signals:
    void traceEntryReceived( const TraceEntry &e );
    void processShutdown( const ProcessShutdownEvent &e );

private:
    void storeEntry( const TraceEntry &e );
    void storeShutdownEvent( const ProcessShutdownEvent &ev );
    void handleDatagram( const QByteArray &datagram );
    void handleTraceEntryXMLData( const QDomDocument &doc );
    void handleShutdownXMLData( const QDomDocument &doc );

    template <typename T>
    QString formatValue( const T &v ) const;

    ServerSocket *m_tcpServer;
    QSqlDatabase m_db;
    QList<NetworkingThread *> m_networkingThreads;
};

#endif // !defined(TRACE_SERVER_H)

