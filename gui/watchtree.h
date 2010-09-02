/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef WATCHTREE_H
#define WATCHTREE_H

#include <QSqlDatabase>
#include <QTreeWidget>
#include <QMap>

struct TraceEntry;

struct TreeItem;
typedef QMap<QString, TreeItem *> ItemMap;

struct TreeItem {
    TreeItem( QTreeWidgetItem *i ) : item( i ) { }

    QTreeWidgetItem *item;
    ItemMap children;
};

class WatchTree : public QTreeWidget
{
    Q_OBJECT
public:
    WatchTree( QWidget *parent = 0 );
    virtual ~WatchTree();

    bool setDatabase( const QString &databaseFileName,
                      QString *errMsg );

public slots:
    void suspend();
    void resume();
    void handleNewTraceEntry( const TraceEntry &e );

private slots:
    bool showNewTraceEntries( QString *errMsg = 0 );

private:
    ItemMap m_applicationItems;
    QSqlDatabase m_db;
    QTimer *m_databasePollingTimer;
    bool m_dirty;
    bool m_suspended;
};

#endif // !defined(WATCHTREE_H)

