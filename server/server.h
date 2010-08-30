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

class QTcpServer;
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
    enum { StringType } type;
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

class Server : public QObject
{
    Q_OBJECT
public:
    Server( const QString &databaseFileName, unsigned short port,
            QObject *parent = 0 );

signals:
    void traceEntryReceived( const TraceEntry &e );
    void processShutdown( const ProcessShutdownEvent &e );

private slots:
    void handleNewConnection();
    void handleIncomingData();

private:
    void storeEntry( const TraceEntry &e );
    void storeShutdownEvent( const ProcessShutdownEvent &ev );
    void handleDatagram( const QByteArray &datagram );
    void handleTraceEntryXMLData( const QDomDocument &doc );
    void handleShutdownXMLData( const QDomDocument &doc );

    template <typename T>
    QString formatValue( const T &v ) const;

    QTcpServer *m_tcpServer;
    QSqlDatabase m_db;
};

#endif // !defined(TRACE_SERVER_H)

