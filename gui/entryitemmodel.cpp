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

#include "entryitemmodel.h"

#include "entryfilter.h"
#include "columnsinfo.h"
#include "../hooklib/tracelib.h"
#ifdef HAVE_MODELTEST
#  include "modeltest.h"
#endif

#include <assert.h>

#include <QBrush>
#include <QDateTime>
#include <QDebug>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTimer>
#include <cassert>

// #define DEBUG_MODEL

#ifdef DEBUG_MODEL
#  include <QTime>
#endif

typedef QVariant (*DataFormatter)(QSqlDatabase db, const EntryItemModel *model, int row, int column);

static QVariant timeFormatter(QSqlDatabase, const EntryItemModel *model, int row, int column)
{
    return QDateTime::fromMSecsSinceEpoch( model->getValue(row, column).toLongLong() );
}

static QString tracePointTypeAsString(int i)
{
    // ### could do some caching here
    // ### assert range - just in case
    using TRACELIB_NAMESPACE_IDENT(TracePointType);
    TracePointType::Value t =  static_cast<TracePointType::Value>(i);
    QString s = TracePointType::valueAsString(t);
    return s;
}

static QVariant typeFormatter(QSqlDatabase, const EntryItemModel *model, int row, int column)
{
    bool ok;
    int i = model->getValue(row, column).toInt(&ok);
    assert(ok);
    return tracePointTypeAsString(i);
}

static QVariant stackPositionFormatter(QSqlDatabase, const EntryItemModel *model, int row, int column)
{
    bool ok;
    qulonglong i = model->getValue(row, column).toULongLong(&ok);
    assert(ok);
    return QString( "0x%1" ).arg( QString::number( i, 16 ) );
}

static QVariant keyFormatter(QSqlDatabase, const EntryItemModel *model, int row, int column)
{
    bool ok;
    qulonglong i = model->getValue(row, column).toULongLong(&ok);
    assert(ok);
    return model->keyName(i);
}

static const struct {
    const char *name;
    DataFormatter formatterFn;
} g_fields[] = {
    { "Time", timeFormatter },
    { "Application", 0 },
    { "PID", 0 },
    { "Thread", 0 },
    { "File", 0 },
    { "Line", 0 },
    { "Function", 0 },
    { "Type", typeFormatter },
    { "Key", keyFormatter },
    { "Message", 0 },
    { "Stack Position", stackPositionFormatter }
};

EntryItemModel::EntryItemModel(EntryFilter *filter, ColumnsInfo *ci,
                               QObject *parent )
    : QAbstractTableModel(parent),
      m_numMatchingEntries(-1),
      m_numNewEntries(0),
      m_databasePollingTimer(NULL),
      m_suspended(false),
      m_filter(filter),
      m_columnsInfo(ci),
      m_highlightedTraceKeyId(-1)
{
#if defined(DEBUG_MODEL) && defined(HAVE_MODELTEST)
    (void)new ModelTest( this, this );
#endif
    m_databasePollingTimer = new QTimer(this);
    m_databasePollingTimer->setSingleShot(true);
    connect(m_databasePollingTimer, SIGNAL(timeout()), SLOT(insertNewTraceEntries()));
    connect(m_columnsInfo, SIGNAL(changed()), SLOT(updateScannedFieldsList()));
}

EntryItemModel::~EntryItemModel()
{
}

bool EntryItemModel::setDatabase(QSqlDatabase database,
                                 QString *errMsg)
{
    m_databasePollingTimer->stop();
    m_numNewEntries = 0;
    m_numMatchingEntries = -1;
    m_suspended = false;

    m_db = database;
    if (!queryForEntries(errMsg, 0))
        return false;

    return true;
}

bool EntryItemModel::queryForEntries(QString *errMsg, int startRow)
{
#ifdef DEBUG_MODEL
    qDebug() << "EntryItemModel::queryForEntries: startRow = " << startRow;
#endif

    QStringList tablesToSelectFrom;
    tablesToSelectFrom.append("trace_entry");

    QStringList predicates;

    if (!m_filter->application().isEmpty()) {
        tablesToSelectFrom.append("process");
        tablesToSelectFrom.append("traced_thread");

        predicates << "trace_entry.traced_thread_id = traced_thread.id"
                   << "traced_thread.process_id = process.id"
                   << QString("process.name LIKE '%%1%'").arg(m_filter->application());
    }

    if (m_filter->processId() != -1) {
        tablesToSelectFrom.append("process");
        tablesToSelectFrom.append("traced_thread");

        predicates << "trace_entry.traced_thread_id = traced_thread.id"
                   << "traced_thread.process_id = process.id"
                   << QString("process.id = %1").arg(m_filter->processId());
    }

    if (m_filter->threadId() != -1) {
        tablesToSelectFrom.append("traced_thread");

        predicates << "trace_entry.traced_thread_id = traced_thread.id"
                   << QString("traced_thread.tid = %1").arg(m_filter->threadId());
    }

    if (!m_filter->function().isEmpty()) {
        tablesToSelectFrom.append("trace_point");
        tablesToSelectFrom.append("function_name");

        predicates << "trace_entry.trace_point_id = trace_point.id"
                   << "trace_point.function_id = function_name.id"
                   << QString("function_name.name LIKE '%%1%'").arg(m_filter->function());
    }

    if (!m_filter->message().isEmpty()) {
        predicates << QString("trace_entry.message LIKE '%%1%'").arg(m_filter->message());
    }

    if (m_filter->type() != -1) {
        tablesToSelectFrom.append("trace_point");

        predicates << "trace_entry.trace_point_id = trace_point.id"
                   << QString("trace_point.type = %1").arg(m_filter->type());
    }

    if (!m_filter->acceptsEntriesWithoutKey() || !m_filter->inactiveKeys().isEmpty()) {
        tablesToSelectFrom.append("trace_point");

        QString inactiveKeyIdTest;
        if (!m_filter->inactiveKeys().isEmpty()) {
            tablesToSelectFrom.append("trace_point_group");

            QStringList keyPredicates;
            QStringList inactiveKeys = m_filter->inactiveKeys();
            QStringList::ConstIterator it, end = inactiveKeys.end();
            for (it = inactiveKeys.begin(); it != end; ++it) {
                keyPredicates << QString("trace_point_group.name != '%1'").arg(*it);
            }

            inactiveKeyIdTest = QString("(trace_point.group_id = trace_point_group.id "
                                          "AND %1)").arg(keyPredicates.join(" AND "));
        }

        QString withoutKeyIdTest;
        if (m_filter->acceptsEntriesWithoutKey()) {
            withoutKeyIdTest = "(trace_point.group_id = 0)";
        } else {
            withoutKeyIdTest = "(trace_point.group_id != 0)";
        }

        QString keyIdTest = withoutKeyIdTest;
        if (!inactiveKeyIdTest.isEmpty()) {
            if (m_filter->acceptsEntriesWithoutKey()) {
                keyIdTest += " OR ";
            } else {
                keyIdTest += " AND ";
            }
            keyIdTest += inactiveKeyIdTest;
        }

        predicates << "trace_entry.trace_point_id = trace_point.id" << QString("(%1)").arg(keyIdTest);
    }

    tablesToSelectFrom.removeDuplicates();
    predicates.removeDuplicates();

    QString fromAndWhereClause = "FROM ";
    fromAndWhereClause += tablesToSelectFrom.join(", ");
    if (!predicates.isEmpty()) {
        fromAndWhereClause += " WHERE ";
        fromAndWhereClause += predicates.join(" AND ");
    }

    if ( m_numMatchingEntries == -1 ) {
        QString countQuery = QString( "SELECT DISTINCT trace_entry.id %1 ORDER BY trace_entry.id;" ).arg(fromAndWhereClause);
#ifdef DEBUG_MODEL
        QTime t;
        t.start();

        qDebug() << "Recomputing number of matching entries...";
        qDebug() << "Query = " << countQuery;
#endif
        QSqlQuery q(m_db);
        q.setForwardOnly(true);
        if (!q.exec(countQuery)) {
            *errMsg = m_db.lastError().text();
            return false;
        }

        m_idForRow.clear();
        while (q.next()) {
            bool ok;
            m_idForRow.append(q.value(0).toUInt(&ok));
            assert(ok);
        }
        m_numMatchingEntries = m_idForRow.size();
#ifdef DEBUG_MODEL
        qDebug() << "Counted " << m_numMatchingEntries << " matching entries in " << t.elapsed() << "ms";
#endif
        if (m_numMatchingEntries == 0) {
            // bail out early if none of the entries matched
            m_topRow = -1;
            m_data.clear();
            updateHighlightedEntries();
            return true;
        }
    }

    assert(startRow >= 0);
    assert(startRow < m_idForRow.size());

    QStringList fieldsToSelect;
    {
        QList<int> visibleColumns = m_columnsInfo->visibleColumns();
        QList<int>::ConstIterator it, end = visibleColumns.end();
        fieldsToSelect.append("trace_entry.id");
        for (it = visibleColumns.begin(); it != end; ++it) {
            const QString cn = m_columnsInfo->columnName(*it);
            if (cn == "Time") {
                fieldsToSelect.append("trace_entry.timestamp");
            } else if (cn == "Application") {
                fieldsToSelect.append("process.name");
                tablesToSelectFrom.append("traced_thread");
                tablesToSelectFrom.append("process");
                predicates << "trace_entry.traced_thread_id = traced_thread.id"
                           << "traced_thread.process_id = process.id";
            } else if (cn == "PID") {
                fieldsToSelect.append("process.pid");
                tablesToSelectFrom.append("traced_thread");
                tablesToSelectFrom.append("process");
                predicates << "trace_entry.traced_thread_id = traced_thread.id"
                           << "traced_thread.process_id = process.id";
            } else if (cn == "Thread") {
                fieldsToSelect.append("traced_thread.tid");
                tablesToSelectFrom.append("traced_thread");
                predicates << "trace_entry.traced_thread_id = traced_thread.id";
            } else if (cn == "File") {
                fieldsToSelect.append("path_name.name");
                tablesToSelectFrom.append("trace_point");
                tablesToSelectFrom.append("path_name");
                predicates << "trace_entry.trace_point_id = trace_point.id"
                           << "trace_point.path_id = path_name.id";
            } else if (cn == "Line") {
                fieldsToSelect.append("trace_point.line");
                tablesToSelectFrom.append("trace_point");
                predicates << "trace_entry.trace_point_id = trace_point.id";
            } else if (cn == "Function") {
                fieldsToSelect.append("function_name.name");
                tablesToSelectFrom.append("trace_point");
                tablesToSelectFrom.append("function_name");
                predicates << "trace_entry.trace_point_id = trace_point.id"
                           << "trace_point.function_id = function_name.id";
            } else if (cn == "Type") {
                fieldsToSelect.append("trace_point.type");
                tablesToSelectFrom.append("trace_point");
                predicates << "trace_entry.trace_point_id = trace_point.id";
            } else if (cn == "Key") {
                fieldsToSelect.append("trace_point.group_id");
                tablesToSelectFrom.append("trace_point");
                predicates << "trace_entry.trace_point_id = trace_point.id";
            } else if (cn == "Message") {
                fieldsToSelect.append("trace_entry.message");
            } else if (cn == "Stack Position") {
                fieldsToSelect.append("trace_entry.stack_position");
            }
        }
    }

    tablesToSelectFrom.removeDuplicates();
    predicates.removeDuplicates();

    predicates << QString("trace_entry.id >= %1").arg(m_idForRow[startRow]);

    QString statement = "SELECT DISTINCT ";
    statement += fieldsToSelect.join( ", ");
    statement += " FROM ";
    statement += tablesToSelectFrom.join(", ");
    statement += " WHERE ";
    statement += predicates.join(" AND ");
    statement += " ORDER BY trace_entry.id LIMIT 100";

#ifdef DEBUG_MODEL
    QTime t;
    t.start();
    qDebug() << "Selecting data...";
    qDebug() << "Query = " << statement;
#endif

    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    if (!query.exec(statement)) {
        *errMsg = m_db.lastError().text();
        return false;
    }

    {
        m_topRow = startRow;

        m_data.clear();
        m_data.reserve(100);

        const int numFields = query.record().count();

        while (query.next()) {
            QVector<QVariant> row(numFields);
            for (int i = 0; i < numFields; ++i) {
                row[i] = query.value(i);
            }
            m_data.append(row);
        }
    }

#ifdef DEBUG_MODEL
    qDebug() << "Selected " << m_data.size() << " rows in " << t.elapsed() << "ms";
#endif

    updateHighlightedEntries();

    return true;
}

int EntryItemModel::columnCount(const QModelIndex & parent) const
{
    return m_columnsInfo->visibleColumns().count();
}

int EntryItemModel::rowCount(const QModelIndex & parent) const
{
    return std::max( 0, m_numMatchingEntries );
}

QModelIndex EntryItemModel::index(int row, int column,
                                  const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, static_cast<void *>(0));
}

const QVariant &EntryItemModel::getValue(int row, int column) const
{
    assert(row >= 0);
    assert(row < m_numMatchingEntries);
    assert(column >= 0);
    if (row < m_topRow || row >= m_topRow + m_data.size()) {
        QString errMsg;
        const_cast<EntryItemModel *>(this)->queryForEntries(&errMsg, row);
    }
    assert(row >= m_topRow);
    assert(row < m_topRow + m_data.size());
    const QVector<QVariant> &rowData = m_data[row - m_topRow];
    assert(column < rowData.size());
    return m_data[row - m_topRow][column];
}

QVariant EntryItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        // undo possible column reordering 
        if (!m_columnsInfo->isVisible(index.column()))
            return QVariant();
        int realColumn = m_columnsInfo->unmap(index.column());

        int dbField = index.column() + 1; // id field is used in header

        if (g_fields[realColumn].formatterFn)
            return g_fields[realColumn].formatterFn(m_db, this, index.row(), dbField);
        return getValue(index.row(), dbField);
    } else if (role == Qt::ToolTipRole) {
        // Just forward the tool tip request for now to make viewing
        // of cut-off content possible.
        // ### consider displaying additional info. Like start/end
        // ### time of an application.
        // ### supress when nothing valuable to show and not cut off
        return data(index, Qt::DisplayRole);
    } else if (role == Qt::BackgroundRole) {
        unsigned int entryId = const_cast<EntryItemModel * const>(this)->idForIndex(index);
        if ( m_highlightedEntryIds.contains( entryId ) ) {
            return QBrush( Qt::yellow );
        }
    } else if (role == Qt::FontRole) {
        return m_cellFont;
    }

    return QVariant();
}

QVariant EntryItemModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        assert((section >= 0 && section < columnCount()) || !"Invalid section value");
        if (!m_columnsInfo->isVisible(section))
            return QVariant();
        int realSection = m_columnsInfo->unmap(section);
        return tr(g_fields[realSection].name);
    } else if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        //assert((section >= 0 && section < rowCount()) || !"Invalid section value");
        if (!(section >= 0 && section < rowCount()))
            return QVariant();
        return getValue(section, 0);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void EntryItemModel::handleNewTraceEntry(const TraceEntry &e)
{
    // Ignore entries that don't match the current filter
    if (!m_filter->matches(e))
        return;

    ++m_numNewEntries;
    if (!m_suspended && !m_databasePollingTimer->isActive()) {
        m_databasePollingTimer->start(200);
    }
}

void EntryItemModel::suspend()
{
    m_suspended = true;
}

void EntryItemModel::resume()
{
    m_suspended = false;
    insertNewTraceEntries();
}

void EntryItemModel::clear()
{
    beginResetModel();
    m_numNewEntries = 0;
    m_numMatchingEntries = 0;
    endResetModel();
}

unsigned int EntryItemModel::idForIndex(const QModelIndex &index)
{
    bool ok;
    const unsigned int id = getValue(index.row(), 0).toUInt(&ok);
    assert(ok);
    return id;
}

void EntryItemModel::insertNewTraceEntries()
{
    if (m_numNewEntries == 0)
        return;

    beginInsertRows(QModelIndex(), m_numMatchingEntries, m_numMatchingEntries + m_numNewEntries - 1);
    m_numMatchingEntries = -1;
    QString errorMsg;
    if (!queryForEntries(&errorMsg, 0)) {
        qDebug() << "EntryItemModel::insertNewTraceEntries: failed: " << errorMsg;
    }
    endInsertRows();

    m_numNewEntries = 0;
}

void EntryItemModel::reApplyFilter()
{
    m_numMatchingEntries = -1;
    beginResetModel();
    QString errorMsg;
    if (!queryForEntries(&errorMsg, 0)) {
        qDebug() << "EntryItemModel::reApplyFilter: failed: " << errorMsg;
    }
    endResetModel();
}

void EntryItemModel::highlightEntries(const QString &term,
                                      const QStringList &fields,
                                      SearchWidget::MatchType matchType)
{
    if ( term.isEmpty() || fields.isEmpty() ) {
        if ( !m_highlightedEntryIds.isEmpty() ) {
            m_highlightedEntryIds.clear();
            // XXX Is there a more elegant way to have the views repaint
            // their visible range?
            emit dataChanged( createIndex( 0, 0, static_cast<void *>( 0 ) ),
                              createIndex( rowCount() - 1, columnCount() - 1, static_cast<void *>( 0 ) ) );
        }
        return;
    }

    switch ( matchType ) {
        case SearchWidget::StrictMatch:
            m_lastSearchTerm.setPatternSyntax( QRegExp::FixedString );
            break;
        case SearchWidget::WildcardMatch:
            m_lastSearchTerm.setPatternSyntax( QRegExp::Wildcard );
            break;
        case SearchWidget::RegExpMatch:
            m_lastSearchTerm.setPatternSyntax( QRegExp::RegExp );
            break;
    }
    m_lastSearchTerm.setPattern( term );

    m_scannedFieldNames = fields;

    updateScannedFieldsList();

    updateHighlightedEntries();
}

void EntryItemModel::highlightTraceKey(const QString &traceKey)
{
    if ( m_highlightedTraceKey != traceKey ) {
        m_highlightedTraceKey = traceKey;

        QSqlQuery q(m_db);
        q.setForwardOnly(true);
        if (q.exec(QString("SELECT trace_point_group.id FROM trace_point_group WHERE trace_point_group.name = '%1'").arg(traceKey))) {
            q.next();
            m_highlightedTraceKeyId = q.value(0).toInt();
        }
        updateHighlightedEntries();
    }
}

void EntryItemModel::updateHighlightedEntries()
{
    QSet<unsigned int> entriesToHighlight;

    int traceKeyColumn = -1;
    if ( !m_highlightedTraceKey.isEmpty() ) {
        const QList<int> visibleColumns = m_columnsInfo->visibleColumns();
        QList<int>::ConstIterator it, end = visibleColumns.end();
        int pos = -1;
        for ( it = visibleColumns.begin(); it != end; ++it, ++pos ) {
            if ( m_columnsInfo->columnCaption( *it ) == tr( "Key" ) ) {
                traceKeyColumn = pos + 1;
                break;
            }
        }
    }

    QVector<QVector<QVariant> >::ConstIterator it, end = m_data.end();
    for ( it = m_data.begin(); it != end; ++it ) {
        const QVector<QVariant> &row = *it;

        bool ok;
        const unsigned int entryId = row[0].toUInt(&ok);
        assert(ok);

        QList<int>::ConstIterator fieldIdxIt, fieldIdxEnd = m_scannedFields.end();
        for ( fieldIdxIt = m_scannedFields.begin(); fieldIdxIt != fieldIdxEnd; ++fieldIdxIt ) {
            const QVariant &v = row[*fieldIdxIt + 1];
            if ( m_lastSearchTerm.exactMatch( v.toString() ) ) {
                entriesToHighlight.insert(entryId);
            }
        }

        if ( traceKeyColumn != -1 ) {
            const QVariant &v = row[traceKeyColumn + 1];
            if ( v.toInt() == m_highlightedTraceKeyId ) {
                entriesToHighlight.insert(entryId);
            }
        }
    }

    if ( entriesToHighlight != m_highlightedEntryIds ) {
        m_highlightedEntryIds = entriesToHighlight;

        // XXX Is there a more elegant way to have the views repaint
        // their visible range?
        emit dataChanged( createIndex( 0, 0, static_cast<void *>( 0 ) ),
                          createIndex( rowCount() - 1, columnCount() - 1, static_cast<void *>( 0 ) ) );
    }
}

void EntryItemModel::updateScannedFieldsList()
{
    m_scannedFields.clear();

    const QList<int> visibleColumns = m_columnsInfo->visibleColumns();
    QList<int>::ConstIterator it, end = visibleColumns.end();
    unsigned int pos = 0;
    for ( it = visibleColumns.begin(); it != end; ++it, ++pos ) {
        if ( m_scannedFieldNames.contains( m_columnsInfo->columnCaption( *it ) ) ) {
            m_scannedFields.append(pos);
        }
    }
}

QString EntryItemModel::keyName(int id) const
{
    if (id == 0) {
        return QString("<None>");
    }
    QSqlQuery q(m_db);
    q.setForwardOnly(true);
    if (!q.exec(QString("SELECT trace_point_group.name FROM trace_point_group WHERE trace_point_group.id = %1").arg(id))) {
        return "";
    }
    q.next();
    return q.value(0).toString();
}

void EntryItemModel::setCellFont(const QFont &font)
{
    m_cellFont = font;
}

