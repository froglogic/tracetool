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
#include <QTcpSocket>
#include <QThread>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

#include "../hooklib/tracelib.h"

class QSignalMapper;

struct StackFrame
{
    QString module;
    QString function;
    size_t functionOffset;
    QString sourceFile;
    size_t lineNumber;
};

QDataStream &operator<<( QDataStream &stream, const StackFrame &entry );
QDataStream &operator>>( QDataStream &stream, StackFrame &entry );

struct Variable
{
    QString name;
    TRACELIB_NAMESPACE_IDENT(VariableType)::Value type;
    QString value;
};

QDataStream &operator<<( QDataStream &stream, const Variable &entry );
QDataStream &operator>>( QDataStream &stream, Variable &entry );

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
    QString groupName;
    QString function;
    QString message;
    QList<Variable> variables;
    QList<StackFrame> backtrace;
    unsigned long stackPosition;
};

QDataStream &operator<<( QDataStream &stream, const TraceEntry &entry );
QDataStream &operator>>( QDataStream &stream, TraceEntry &entry );

struct ProcessShutdownEvent
{
    unsigned int pid;
    QDateTime startTime;
    QDateTime stopTime;
    QString name;
};

QDataStream &operator<<( QDataStream &stream, const ProcessShutdownEvent &ev );
QDataStream &operator>>( QDataStream &stream, ProcessShutdownEvent &ev );

struct TracedApplicationInfo
{
    unsigned int pid;
    QDateTime startTime;
    QDateTime stopTime;
    QString name;
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

extern const quint32 MagicServerProtocolCookie;
enum ServerDatagramType {
    TraceFileNameDatagram,
    TraceEntryDatagram,
    ProcessShutdownEventDatagram
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

    void trimTo( size_t nMostRecent );

    QList<StackFrame> backtraceForEntry( unsigned int id );
    void addGroupId( const QString &id );
    QStringList seenGroupIds() const;
    QList<TracedApplicationInfo> tracedApplications() const;

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

    template <typename T>
    QString formatValue( const T &v ) const;

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

