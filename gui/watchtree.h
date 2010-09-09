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
class EntryFilter;

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
    WatchTree(EntryFilter *filter, QWidget *parent = 0);
    virtual ~WatchTree();

    bool setDatabase( const QString &databaseFileName,
                      QString *errMsg );

public slots:
    void suspend();
    void resume();
    void handleNewTraceEntry( const TraceEntry &e );
    void reApplyFilter();

private slots:
    void showNewTraceEntriesFireAndForget();

private:
    bool showNewTraceEntries( QString *errMsg );
    ItemMap m_applicationItems;
    QSqlDatabase m_db;
    QTimer *m_databasePollingTimer;
    bool m_dirty;
    bool m_suspended;
    EntryFilter *m_filter;
};

#endif // !defined(WATCHTREE_H)

