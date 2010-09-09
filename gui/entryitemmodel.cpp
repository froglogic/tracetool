/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "entryitemmodel.h"

#include "entryfilter.h"
#include "columnsinfo.h"
#include "../hooklib/tracelib.h"

#include <QBrush>
#include <QDateTime>
#include <QDebug>
#include <QSqlDriver>
#include <QSqlError>
#include <QTimer>
#include <cassert>

// #define SHOW_VERBOSITY

typedef QVariant (*DataFormatter)( const QVariant &v );

static QVariant timeFormatter( const QVariant &v )
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

static QVariant typeFormatter( const QVariant &v )
{
    bool ok;
    int i = v.toInt(&ok);
    assert(ok);
    return tracePointTypeAsString(i);
}

static QVariant stackPositionFormatter( const QVariant &v )
{
    bool ok;
    qulonglong i = v.toULongLong(&ok);
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
      m_numNewEntries(0),
      m_databasePollingTimer(NULL),
      m_suspended(false),
      m_filter(filter),
      m_columnsInfo(ci)
{
    m_databasePollingTimer = new QTimer(this);
    m_databasePollingTimer->setSingleShot(true);
    connect(m_databasePollingTimer, SIGNAL(timeout()), SLOT(insertNewTraceEntries()));
    connect(m_filter, SIGNAL(changed()), SLOT(reApplyFilter()));
}

EntryItemModel::~EntryItemModel()
{
}

bool EntryItemModel::setDatabase(const QString &databaseFileName,
                                 QString *errMsg)
{
    const QString driverName = "QSQLITE";
    if (!QSqlDatabase::isDriverAvailable(driverName)) {
        *errMsg = tr("Missing required %1 driver.").arg(driverName);
        return false;
    }

    m_db = QSqlDatabase::addDatabase(driverName, "itemmodel");
    m_db.setDatabaseName(databaseFileName);
    if (!m_db.open()) {
        *errMsg = m_db.lastError().text();
        return false;
    }

    if (!queryForEntries(errMsg))
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

bool EntryItemModel::queryForEntries(QString *errMsg)
{
    // ### respect hidden columns might speed things up
    m_query = m_db.exec("SELECT"
                        " trace_entry.id,"
                        " timestamp,"
                        " process.name,"
                        " process.pid,"
                        " traced_thread.tid,"
                        " path_name.name,"
                        " trace_point.line,"
                        " function_name.name,"
                        " trace_point.type,"
#ifdef SHOW_VERBOSITY
                        " trace_point.verbosity,"
#endif
                        " message,"
                        " trace_entry.stack_position "
                        "FROM"
                        " trace_entry,"
                        " trace_point,"
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
                        filterClause(m_filter) +
                        "ORDER BY"
                        " trace_entry.id");

    if (m_db.lastError().isValid()) {
        *errMsg = m_db.lastError().text();
        return false;
    }

    if (m_query.driver()->hasFeature(QSqlDriver::QuerySize)) {
        m_querySize = m_query.size();
    } else {
        m_querySize = 0;
        while (m_query.next()) {
            ++m_querySize;
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
    return m_querySize;
}

QModelIndex EntryItemModel::index(int row, int column,
                                  const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column, 0);
}

QVariant EntryItemModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole) {
        // undo possible column reordering 
        if (!m_columnsInfo->isVisible(index.column()))
            return QVariant();
        int realColumn = m_columnsInfo->unmap(index.column());
        int dbField = realColumn + 1; // id field is used in header
        const_cast<EntryItemModel*>(this)->m_query.seek(index.row());
        QVariant v = m_query.value(dbField);
        if ( g_fields[realColumn].formatterFn )
            return g_fields[realColumn].formatterFn( v );
        return v;
    } else if (role == Qt::ToolTipRole) {
        // Just forward the tool tip request for now to make viewing
        // of cut-off content possible.
        // ### consider displaying additional info. Like start/end
        // ### time of an application.
        // ### supress when nothing valuable to show and not cut off
        return data(index, Qt::DisplayRole);
    } else if (role == Qt::BackgroundRole) {
        const_cast<EntryItemModel*>(this)->m_query.seek(index.row());
        unsigned int entryId = m_query.value(0).toUInt();
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
        const_cast<EntryItemModel*>(this)->m_query.seek(section);
        return m_query.value(0);
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

void EntryItemModel::insertNewTraceEntries()
{
    if (m_numNewEntries == 0)
        return;

    beginInsertRows(QModelIndex(), m_querySize, m_querySize + m_numNewEntries - 1);
    QString errorMsg;
    if (!queryForEntries(&errorMsg)) {
        qDebug() << "EntryItemModel::insertNewTraceEntries: failed: " << errorMsg;
    }
    endInsertRows();

    m_numNewEntries = 0;
}

void EntryItemModel::reApplyFilter()
{
    beginResetModel();
    QString errorMsg;
    if (!queryForEntries(&errorMsg)) {
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

