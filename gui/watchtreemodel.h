/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef WATCHTREEMODEL_H
#define WATCHTREEMODEL_H

#include <QObject>

class WatchTreeModel : public QObject
{
    Q_OBJECt
public:
    WatchTreeModel(QObject *parent = 0);

    void suspend();
    void resume();

private slots:
    void handleNewTraceEntry(const TraceEntry &e);
    void insertNewTraceEntries();

private:
    bool queryForEntries(QString *errMsg);

    QSqlDatabase m_db;
    QSqlQuery m_query;
    int m_querySize;
    Server *m_server;
    unsigned int m_numNewEntries;
    QTimer *m_databasePollingTimer;
    bool m_suspended;
};

#endif // !defined(WATCHTREEMODEL_H)

