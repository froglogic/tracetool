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
    void clear();

    unsigned int idForIndex(const QModelIndex &index);
    const QVariant &getValue(int row, int column) const;

    QString keyName(int id) const;

    void setCellFont(const QFont &font);

public slots:
    void handleNewTraceEntry(const TraceEntry &e);
    void reApplyFilter();
    void highlightEntries(const QString &term,
                          const QStringList &fields,
                          SearchWidget::MatchType matchType);
    void highlightTraceKey(const QString &key);

private slots:
    void insertNewTraceEntries();
    void updateScannedFieldsList();

private:
    bool queryForEntries(QString *errMsg, int startRow);
    void updateHighlightedEntries();

    QSqlDatabase m_db;
    int m_numMatchingEntries;
    int m_topRow;
    QVector<QVector<QVariant> > m_data;
    QVector<unsigned int> m_idForRow;
    unsigned int m_numNewEntries;
    QTimer *m_databasePollingTimer;
    bool m_suspended;
    EntryFilter *m_filter;
    ColumnsInfo *m_columnsInfo;
    QSet<unsigned int> m_highlightedEntryIds;
    QRegExp m_lastSearchTerm;
    QStringList m_scannedFieldNames;
    QList<int> m_scannedFields;
    QString m_highlightedTraceKey;
    int m_highlightedTraceKeyId;
    QFont m_cellFont;
};

#endif
