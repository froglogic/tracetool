/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "entryitemmodel.h"

#include "entryfilter.h"
#include "columnsinfo.h"
#include "../hooklib/tracelib.h"

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

// #define SHOW_VERBOSITY
// #define DEBUG_MODEL

typedef QVariant (*DataFormatter)(QSqlDatabase db, const EntryItemModel *model, int row, int column);

static QVariant timeFormatter(QSqlDatabase, const EntryItemModel *model, int row, int column)
{
    return QDateTime::fromString(model->getValue(row, column).toString(), Qt::ISODate);
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
#ifdef SHOW_VERBOSITY
    { "Verbosity", 0 },
#endif
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
      m_columnsInfo(ci)
{
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
    m_db = database;
    if (!queryForEntries(errMsg, 0))
        return false;

    return true;
}

static QString filterClause(EntryFilter *f)
{
    QString sql = f->whereClause("process.name",
                                 "process.pid",
                                 "traced_thread.tid",
                                 "function_name.name",
                                 "message",
                                 "trace_point.type");
    if (sql.isEmpty())
        return QString();

    return "AND " + sql + " ";
}

bool EntryItemModel::queryForEntries(QString *errMsg, int startRow)
{
#ifdef DEBUG_MODEL
    qDebug() << "EntryItemModel::queryForEntries: startRow = " << startRow;
#endif

    QString fromAndWhereClause =
                        "FROM"
                        " trace_entry,"
                        " trace_point,";
    if (!m_filter->inactiveKeys().isEmpty())
        fromAndWhereClause +=
                        " trace_point_group,";
    fromAndWhereClause +=
                        " path_name, "
                        " function_name, "
                        " process, "
                        " traced_thread "
                        "WHERE"
                        " trace_entry.trace_point_id = trace_point.id "
                        "AND"
                        " trace_point.function_id = function_name.id "
                        "AND"
                        " trace_point.path_id = path_name.id "
                        "AND"
                        " trace_entry.traced_thread_id = traced_thread.id "
                        "AND"
                        " traced_thread.process_id = process.id " +
                        filterClause(m_filter);

    if ( m_numMatchingEntries == -1 ) {
#ifdef DEBUG_MODEL
        qDebug() << "Recomputing number of matching entries...";
#endif
        QSqlQuery q(m_db);
        q.setForwardOnly(true);
        if (!q.exec(QString( "SELECT trace_entry.id %1 ORDER BY trace_entry.id;" ).arg(fromAndWhereClause))) {
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
            } else if (cn == "PID") {
                fieldsToSelect.append("process.pid");
            } else if (cn == "Thread") {
                fieldsToSelect.append("traced_thread.tid");
            } else if (cn == "File") {
                fieldsToSelect.append("path_name.name");
            } else if (cn == "Line") {
                fieldsToSelect.append("trace_point.line");
            } else if (cn == "Function") {
                fieldsToSelect.append("function_name.name");
            } else if (cn == "Type") {
                fieldsToSelect.append("trace_point.type");
            } else if (cn == "Verbosity") {
                fieldsToSelect.append("trace_point.verbosity");
            } else if (cn == "Message") {
                fieldsToSelect.append("trace_entry.message");
            } else if (cn == "Stack Position") {
                fieldsToSelect.append("trace_entry.stack_position");
            }
        }
    }

    QString statement = "SELECT ";
    statement += fieldsToSelect.join( ", ");
    statement += " %1 AND trace_entry.id >= %2 ORDER BY trace_entry.id LIMIT 100";

    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    if (!query.exec(statement.arg(fromAndWhereClause).arg(m_idForRow[startRow]))) {
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

    updateHighlightedEntries();

    return true;
}

int EntryItemModel::columnCount(const QModelIndex & parent) const
{
    return static_cast<int>( sizeof(g_fields) / sizeof(g_fields[0]) );
}

int EntryItemModel::rowCount(const QModelIndex & parent) const
{
    return m_numMatchingEntries;
}

QModelIndex EntryItemModel::index(int row, int column,
                                  const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, 0);
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
        return getValue(section, 0);
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

void EntryItemModel::handleNewTraceEntry(const TraceEntry &e)
{
    // Ignore entries that don't match the current filter
    if (!m_filter->matches(e))
        return;

    m_numMatchingEntries = -1;
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
            emit dataChanged( createIndex( 0, 0, 0 ),
                              createIndex( rowCount() - 1, columnCount() - 1, 0 ) );
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

void EntryItemModel::updateHighlightedEntries()
{
    QSet<unsigned int> entriesToHighlight;

    QVector<QVector<QVariant> >::ConstIterator it, end = m_data.end();
    for ( it = m_data.begin(); it != end; ++it ) {
        const QVector<QVariant> &row = *it;

        QList<int>::ConstIterator fieldIdxIt, fieldIdxEnd = m_scannedFields.end();
        for ( fieldIdxIt = m_scannedFields.begin(); fieldIdxIt != fieldIdxEnd; ++fieldIdxIt ) {
            const QVariant &v = row[*fieldIdxIt + 1];
            if ( m_lastSearchTerm.exactMatch( v.toString() ) ) {
                bool ok;
                const unsigned int entryId = row[0].toUInt(&ok);
                assert(ok);
                entriesToHighlight.insert(entryId);
            }
        }
    }

    if ( entriesToHighlight != m_highlightedEntryIds ) {
        m_highlightedEntryIds = entriesToHighlight;

        // XXX Is there a more elegant way to have the views repaint
        // their visible range?
        emit dataChanged( createIndex( 0, 0, 0 ),
                          createIndex( rowCount() - 1, columnCount() - 1, 0 ) );
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

