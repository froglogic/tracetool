/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "entryitemmodel.h"

#include "entryfilter.h"
#include "../core/tracelib.h"

#include <QDateTime>
#include <QDebug>
#include <QSqlDriver>
#include <QSqlError>
#include <QTimer>
#include <cassert>

// #define SHOW_VERBOSITY

const int IdFieldIndex = 0;
const int TimeFieldIndex = 1;
const int ApplicationFieldIndex = 2;
const int PIDFieldIndex = 3;
const int ThreadFieldIndex = 4;
const int FileFieldIndex = 5;
const int LineFieldIndex = 6;
const int FunctionFieldIndex = 7;
const int TypeFieldIndex = 8;
#ifdef SHOW_VERBOSITY
const int VerbosityFieldIndex = 9;
const int MessageFieldIndex = 10;
const int StackPositionFieldIndex = 11;
#else
const int MessageFieldIndex = 9;
const int StackPositionFieldIndex = 10;
#endif

static QString tracePointTypeAsString(int i)
{
    // ### could do some caching here
    // ### assert range - just in case
    using TRACELIB_NAMESPACE_IDENT(TracePointType);
    TracePointType::Value t =  static_cast<TracePointType::Value>(i);
    QString s = TracePointType::valueAsString(t);
    return s;
}

EntryItemModel::EntryItemModel(EntryFilter *filter, QObject *parent )
    : QAbstractTableModel(parent),
      m_numNewEntries(0),
      m_databasePollingTimer(NULL),
      m_suspended(false),
      m_filter(filter)

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
    QString sql;
    // ### implement for other criteria
    // ### take care of escaping
    if (!f->application().isEmpty()) {
        sql += QString("process.name LIKE '%%1%'").arg(f->application());
    }

    if (sql.isEmpty())
        return sql;

    return "AND " + sql + " ";
}

bool EntryItemModel::queryForEntries(QString *errMsg)
{
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
#ifdef SHOW_VERBOSITY
    return 11;
#else
    return 10;
#endif
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
        int dbField = index.column() + 1;
        const_cast<EntryItemModel*>(this)->m_query.seek(index.row());
        QVariant v = m_query.value(dbField);
        //  qDebug("v.type: %s v.str: %s", v.typeName(), qPrintable(v.toString()));
        // ### Sqlite stores DATETIME values as strings
        if (dbField == TimeFieldIndex) {
            QDateTime dt = QDateTime::fromString(v.toString(), Qt::ISODate);
            return dt;
        }
        if (dbField == StackPositionFieldIndex) {
            bool ok;
            qulonglong i = v.toULongLong(&ok);
            assert(ok);
            return QString( "0x%1" ).arg( QString::number( i, 16 ) );
        }
        if (dbField == TypeFieldIndex) {
            bool ok;
            int i = v.toInt(&ok);
            assert(ok);
            return tracePointTypeAsString(i);
        }
        return v;
    } else if (role == Qt::ToolTipRole) {
        // Just forward the tool tip request for now to make viewing
        // of cut-off content possible.
        // ### consider displaying additional info. Like start/end
        // ### time of an application.
        // ### supress when nothing valuable to show and not cut off
        return data(index, Qt::DisplayRole);
    }

    return QVariant();
}

QVariant EntryItemModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        int dbField = section + 1;
        switch (dbField) {
        case TimeFieldIndex:
            return tr("Time");
        case ApplicationFieldIndex:
            return tr("Application");
        case PIDFieldIndex:
            return tr("PID");
        case ThreadFieldIndex:
            return tr("Thread");
        case FileFieldIndex:
            return tr("File");
        case LineFieldIndex:
            return tr("Line");
        case FunctionFieldIndex:
            return tr("Function");
        case TypeFieldIndex:
            return tr("Type");
#ifdef SHOW_VERBOSITY
        case VerbosityFieldIndex:
            return tr("Verbosity");
#endif
        case MessageFieldIndex:
            return tr("Message");
        case StackPositionFieldIndex:
            return tr("Stack Position");
        default:
            assert(!"Invalid section value");
            return QString();
        }
    } else if (role == Qt::DisplayRole && orientation == Qt::Vertical) {
        const_cast<EntryItemModel*>(this)->m_query.seek(section);
        return m_query.value(IdFieldIndex);
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

