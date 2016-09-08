/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    bool setDatabase( QSqlDatabase database,
                      QString *errMsg );

public slots:
    void suspend();
    void resume();
    void handleNewTraceEntry( const TraceEntry &e );
    void reApplyFilter();

protected:
    virtual void showEvent(QShowEvent *e);

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

