/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef ENTRYITEMMODEL_H
#define ENTRYITEMMODEL_H

#include <QAbstractTableModel>
#include <QSqlDatabase>
#include <QSqlQuery>
class QTimer;

struct TraceEntry;
class EntryFilter;

class EntryItemModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    EntryItemModel(EntryFilter *filter, QObject *parent = 0);
    ~EntryItemModel();

    bool setDatabase(const QString &databaseFileName,
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

public slots:
    void handleNewTraceEntry(const TraceEntry &e);
    void reApplyFilter();

private slots:
    void insertNewTraceEntries();

private:
    bool queryForEntries(QString *errMsg);

    QSqlDatabase m_db;
    QSqlQuery m_query;
    int m_querySize;
    unsigned int m_numNewEntries;
    QTimer *m_databasePollingTimer;
    bool m_suspended;
    EntryFilter *m_filter;
};

#endif
