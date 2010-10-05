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

typedef QVariant (*DataFormatter)(QSqlDatabase db, const QVariant &v);

static QVariant timeFormatter(QSqlDatabase, const QVariant &v)
{
    return QDateTime::fromString(v.toString(), Qt::ISODate);
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

static QVariant typeFormatter(QSqlDatabase, const QVariant &v)
{
    bool ok;
    int i = v.toInt(&ok);
    assert(ok);
    return tracePointTypeAsString(i);
}

static QVariant stackPositionFormatter(QSqlDatabase, const QVariant &v)
{
    bool ok;
    qulonglong i = v.toULongLong(&ok);
    assert(ok);
    return QString( "0x%1" ).arg( QString::number( i, 16 ) );
}

static QString variablesForEntryId(QSqlDatabase db, unsigned int id)
{
    QSqlQuery q = db.exec(QString(
                "SELECT"
                " name,"
                " value,"
                " type "
                "FROM"
                " variable "
                "WHERE"
                " trace_entry_id=%1;").arg(id));
    if (db.lastError().isValid()) {
        qWarning() << "Failed to get variables for trace entry id " << id
            << ": " << db.lastError().text();
        return QString();
    }

    QStringList items;
    while (q.next()) {
        const QString varName = q.value(0).toString();

        QString varValue = q.value(1).toString();
        using TRACELIB_NAMESPACE_IDENT(VariableType);
        const VariableType::Value varType = static_cast<VariableType::Value>(q.value(2).toInt());
        if (varType == VariableType::String) {
            varValue = QString("'%1'").arg(varValue);
        }

        items << QString("%1 (%2) = %3")
                    .arg(q.value(0).toString())
                    .arg(VariableType::valueAsString(varType))
                    .arg(varValue);
    }
    return items.join(", ");
}

static QVariant variablesFormatter(QSqlDatabase db, const QVariant &v)
{
    bool ok;
    unsigned int traceEntryId = v.toUInt(&ok);
    assert(ok);
    return variablesForEntryId(db, traceEntryId);
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
    { "Stack Position", stackPositionFormatter },
    { "Variables", variablesFormatter }
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


    QString statement = "SELECT ";
    statement += fieldsToSelect.join( ", ");
    statement += " %1 ORDER BY trace_entry.id";
    statement += " LIMIT 100 OFFSET %2";

    QSqlQuery query(m_db);
    query.setForwardOnly(true);
    if (!query.exec(statement.arg(fromAndWhereClause).arg(startRow))) {
        *errMsg = m_db.lastError().text();
        return false;
    }

    if ( m_numMatchingEntries == -1 ) {
#ifdef DEBUG_MODEL
        qDebug() << "Recomputing number of matching entries...";
#endif
        if (query.driver()->hasFeature(QSqlDriver::QuerySize)) {
            m_numMatchingEntries = query.size();
        } else {
            QSqlQuery q = m_db.exec(QString( "SELECT COUNT(*) %1;" ).arg(fromAndWhereClause));
            q.next();
            m_numMatchingEntries = q.value(0).toInt();
        }
    }

    {
        m_topRow = startRow;

        m_data.clear();
        m_data.reserve(100);

        const int numFields = query.record().count();

        while (query.next() ) {
            QVector<QVariant> row(numFields);
            for (int i = 0; i < numFields; ++i) {
                row[i] = query.value(i);
            }
            m_data.append(row);
        }
    }

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

        const QVariant v = getValue(index.row(), dbField);

        if (g_fields[realColumn].formatterFn)
            return g_fields[realColumn].formatterFn(m_db, v);
        return v;
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

    QSet<unsigned int> entriesToHighlight;

    QStringList fieldConstraints;

    QString valueTestCode;
    switch ( matchType ) {
        case SearchWidget::StrictMatch:
            valueTestCode = QString( "= '%1'" ).arg( term );
            break;
        case SearchWidget::WildcardMatch:
            valueTestCode = QString( "LIKE '%1'" ).arg( term );
            valueTestCode = valueTestCode.replace( '*', "%" );
            valueTestCode = valueTestCode.replace( '.', '_' );
            break;
        case SearchWidget::RegExpMatch:
            // XXX Using the REGEXP statement requires registering a
            // user-defined 'regexp' function via the SQLite API (see
            // http://www.sqlite.org/c3ref/create_function.html).
            valueTestCode = QString( "REGEXP '%1'" ).arg( term );
            break;
    }

    QStringList::ConstIterator it, end = fields.end();
    for ( it = fields.begin(); it != end; ++it ) {
        if ( *it == tr( "Application" ) ) {
            fieldConstraints <<
                QString( "(traced_thread.id = trace_entry.traced_thread_id AND "
                         " traced_thread.process_id = process.id AND"
                         " process.name %1)" )
                    .arg( valueTestCode );
        } else if ( *it == tr( "File" ) ) {
            fieldConstraints <<
                QString( "(trace_point.id = trace_entry.trace_point_id AND"
                         " path_name.id = trace_point.path_id AND "
                         " path_name.name %1)" )
                    .arg( valueTestCode );
        } else if ( *it == tr( "Function" ) ) {
            fieldConstraints <<
                QString( "(trace_point.id = trace_entry.trace_point_id AND"
                         " function_name.id = trace_point.function_id AND"
                         " function_name.name %1)" )
                    .arg( valueTestCode );
        } else if ( *it == tr( "Message" ) ) {
            fieldConstraints <<
                QString( "(trace_entry.message %1)" )
                    .arg( valueTestCode );
        }
    }

    // XXX Make this query respect the configured filter for
    // performance reasons
    QString query = "SELECT"
                    " DISTINCT trace_entry.id "
                    "FROM"
                    " trace_entry,"
                    " trace_point,"
                    " path_name,"
                    " function_name,"
                    " traced_thread,"
                    " process "
                    "WHERE ";
    query += fieldConstraints.join( " OR " );

    QSqlQuery filterQuery( m_db );
    if ( !filterQuery.exec( query ) ) {
        fprintf( stderr, "ERROR: %s\n", filterQuery.lastError().text().toLatin1().data() );
    }

    while ( filterQuery.next() ) {
        entriesToHighlight.insert( filterQuery.value( 0 ).toUInt() );
    }

    if ( entriesToHighlight != m_highlightedEntryIds ) {
        m_highlightedEntryIds = entriesToHighlight;

        // XXX Is there a more elegant way to have the views repaint
        // their visible range?
        emit dataChanged( createIndex( 0, 0, 0 ),
                          createIndex( rowCount() - 1, columnCount() - 1, 0 ) );
    }
}

