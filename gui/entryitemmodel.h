/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef ENTRYITEMMODEL_H
#define ENTRYITEMMODEL_H

#include "searchwidget.h"

#include <QAbstractTableModel>
#include <QSet>
#include <QSqlDatabase>

class QTimer;

struct TraceEntry;
class EntryFilter;
class ColumnsInfo;

class EntryItemModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    EntryItemModel(EntryFilter *filter, ColumnsInfo *ci, QObject *parent = 0);
    ~EntryItemModel();

    bool setDatabase(QSqlDatabase database,
                     QString *errMsg);

    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    void suspend();
    void resume();

    unsigned int idForIndex(const QModelIndex &index);

public slots:
    void handleNewTraceEntry(const TraceEntry &e);
    void reApplyFilter();
    void highlightEntries(const QString &term,
                          const QStringList &fields,
                          SearchWidget::MatchType matchType);

private slots:
    void insertNewTraceEntries();

private:
    bool queryForEntries(QString *errMsg, int startRow);
    const QVariant &getValue(int row, int column) const;

    QSqlDatabase m_db;
    int m_numMatchingEntries;
    int m_topRow;
    QVector<QVector<QVariant> > m_data;
    unsigned int m_numNewEntries;
    QTimer *m_databasePollingTimer;
    bool m_suspended;
    EntryFilter *m_filter;
    ColumnsInfo *m_columnsInfo;
    QSet<unsigned int> m_highlightedEntryIds;
};

#endif
