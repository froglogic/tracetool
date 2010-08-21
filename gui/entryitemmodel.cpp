#include "entryitemmodel.h"

#include "../server/server.h"

#include <QDateTime>
#include <QSqlError>
#include <QSqlQueryModel>
#include <cassert>

const int IdFieldIndex = 0;
const int TimeFieldIndex = 1;
const int ApplicationFieldIndex = 2;
const int PIDFieldIndex = 3;
const int ThreadFieldIndex = 4;
const int FileFieldIndex = 5;
const int LineFieldIndex = 6;
const int FunctionFieldIndex = 7;
const int TypeFieldIndex = 8;
const int VerbosityFieldIndex = 9;
const int MessageFieldIndex = 10;


EntryItemModel::EntryItemModel(QObject *parent )
    : QAbstractTableModel(parent),
      m_queryModel(new QSqlQueryModel(this)),
      m_server(NULL)

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

    // will create new db file if necessary
    m_server = new Server(databaseFileName, 12382, this);
    
    m_db = QSqlDatabase::addDatabase(driverName, "itemmodel");
    m_db.setDatabaseName(databaseFileName);
    if (!m_db.open()) {
        *errMsg = m_db.lastError().text();
        return false;
    }

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
                        " trace_point.verbosity,"
                        " message "
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
                        " traced_thread.process_id = process.id");

    if (m_db.lastError().isValid()) {
        *errMsg = m_db.lastError().text();
        return false;
    }
    m_queryModel->setQuery(m_query);

    return true;
}

int EntryItemModel::columnCount(const QModelIndex & parent) const
{
    return 11;
}

int EntryItemModel::rowCount(const QModelIndex & parent) const
{
    return m_queryModel->rowCount(QModelIndex());
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
        QModelIndex foreign = m_queryModel->index(index.row(),
                                                  index.column(),
                                                  QModelIndex());
        QVariant v = m_queryModel->data(foreign, Qt::DisplayRole);
        //  qDebug("v.type: %s v.str: %s", v.typeName(), qPrintable(v.toString()));
        // ### Why does Sqlite not return the right type? driver bug?
        if (index.column() == TimeFieldIndex) {
            QDateTime dt;
            dt.setTime_t(v.toLongLong());
            return dt;
        }
        return v;
    }

    return QVariant();
}

QVariant EntryItemModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case IdFieldIndex:
            return tr("Id");
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
        case VerbosityFieldIndex:
            return tr("Verbosity");
        case MessageFieldIndex:
            return tr("Message");
        default:
            assert(!"Invalid section value");
            return QString();
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

