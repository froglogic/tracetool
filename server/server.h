#ifndef TRACE_SERVER_H
#define TRACE_SERVER_H

#include <QObject>
#include <QSqlDatabase>

class QTcpServer;
class QTcpSocket;

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
    QString backtrace;
    QString variables;
};

class Server : public QObject
{
    Q_OBJECT
public:
    Server( QObject *parent, const QString &databaseFileName, unsigned short port );

signals:
    void traceEntryReceived( const TraceEntry &e );

private slots:
    void handleNewConnection();
    void handleIncomingData();

private:
    void storeEntry( const TraceEntry &e );

    QTcpServer *m_tcpServer;
    QSqlDatabase m_db;
};

#endif // !defined(TRACE_SERVER_H)

