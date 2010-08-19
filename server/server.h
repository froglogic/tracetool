#ifndef TRACE_SERVER_H
#define TRACE_SERVER_H

#include <QByteArray>
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
    QString processName;
    unsigned int tid;
    time_t timestamp;
    unsigned int verbosity;
    unsigned int type;
    QString path;
    unsigned long lineno;
    QString function;
    QString message;
    QList<Variable> variables;
    QList<StackFrame> backtrace;
};

class Server : public QObject
{
    Q_OBJECT
public:
    Server( const QString &databaseFileName, unsigned short port,
            QObject *parent = 0 );

signals:
    void traceEntryReceived( const TraceEntry &e );

private slots:
    void handleNewConnection();
    void handleIncomingData();

private:
    void storeEntry( const TraceEntry &e );
    void handleTraceEntryXMLData( const QByteArray &data );

    QTcpServer *m_tcpServer;
    QSqlDatabase m_db;
};

#endif // !defined(TRACE_SERVER_H)

