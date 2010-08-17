#ifndef TRACE_SERVER_H
#define TRACE_SERVER_H

#include <QObject>

class QTcpServer;
class QTcpSocket;
class QSqlDatabase;

struct TraceEntry
{
    unsigned int pid;
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
    Server( QObject *parent, QSqlDatabase *db, unsigned short port );

private slots:
    void handleNewConnection();
    void handleIncomingData();

private:
    QTcpServer *m_tcpServer;
    QSqlDatabase *m_db;
};

#endif // !defined(TRACE_SERVER_H)

