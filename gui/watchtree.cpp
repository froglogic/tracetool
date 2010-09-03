#include "watchtree.h"

#include "entryfilter.h"
#include "../server/server.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>

WatchTree::WatchTree(EntryFilter *filter, QWidget *parent)
    : QTreeWidget( parent ),
    m_databasePollingTimer( 0 ),
    m_dirty( false ),
    m_suspended(false),
    m_filter(filter)
{
    static const char * columns[] = {
        "Position",
        "Name",
        "Value",
        "Old Value"
    };

    setColumnCount( sizeof( columns ) / sizeof( columns[0] ) );
    for ( size_t i = 0; i < sizeof( columns ) / sizeof( columns[0] ); ++i ) {
        headerItem()->setData( i, Qt::DisplayRole, tr( columns[i] ) );
    }

    m_databasePollingTimer = new QTimer(this);
    m_databasePollingTimer->setSingleShot(true);
    connect(m_databasePollingTimer, SIGNAL(timeout()), SLOT(showNewTraceEntries()));
    connect(m_filter, SIGNAL(changed()), SLOT(reApplyFilter()));
}

static void deleteItemMap( ItemMap &m )
{
    ItemMap::Iterator it, end = m.end();
    for ( it = m.begin(); it != end; ++it ) {
        TreeItem *item = *it;
        deleteItemMap( item->children );
        delete item;
    }
}

WatchTree::~WatchTree()
{
    deleteItemMap( m_applicationItems );
}

bool WatchTree::setDatabase( const QString &databaseFileName,
                             QString *errMsg )
{
    const QString driverName = "QSQLITE";
    if (!QSqlDatabase::isDriverAvailable(driverName)) {
        *errMsg = tr("Missing required %1 driver.").arg(driverName);
        return false;
    }

    m_db = QSqlDatabase::addDatabase(driverName, "watchtreemodel");
    m_db.setDatabaseName(databaseFileName);
    if (!m_db.open()) {
        *errMsg = m_db.lastError().text();
        return false;
    }

    m_dirty = true;

    return showNewTraceEntries( errMsg );
}

void WatchTree::suspend()
{
    m_suspended = true;
}

void WatchTree::resume()
{
    m_suspended = false;
    showNewTraceEntries();
}

void WatchTree::handleNewTraceEntry( const TraceEntry &e )
{
    if (!m_filter->matches(e))
        return;
    if ( e.variables.isEmpty() ) {
        return;
    }

    m_dirty = true;
    if ( !m_suspended && !m_databasePollingTimer->isActive() ) {
        m_databasePollingTimer->start( 250 );
    }
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

    return " AND " + sql + " ";
}

bool WatchTree::showNewTraceEntries( QString *errMsg )
{
    if ( !m_dirty ) {
        return true;
    }

    QSqlQuery query( m_db );
    bool result = query.exec( "SELECT"
                "  process.name,"
                "  process.pid,"
                "  path_name.name,"
                "  trace_point.line,"
                "  function_name.name,"
                "  variable.name,"
                "  variable.type,"
                "  variable.value"
                " FROM"
                "  traced_thread,"
                "  process,"
                "  path_name,"
                "  trace_point,"
                "  function_name,"
                "  variable,"
                "  trace_entry"
                " WHERE"
                "  trace_entry.id IN ("
                "    SELECT"
                "      DISTINCT trace_entry_id"
                "    FROM"
                "      variable"
                "    WHERE"
                "      trace_entry_id IN ("
                "        SELECT"
                "          MAX(id)"
                "        FROM"
                "          trace_entry"
                "        GROUP BY trace_point_id, traced_thread_id"
                "      )"
                "  )"
                " AND"
                "  variable.trace_entry_id = trace_entry.id"
                " AND"
                "  traced_thread.id = trace_entry.traced_thread_id"
                " AND"
                "  process.id = traced_thread.process_id"
                " AND"
                "  trace_point.id = trace_entry.trace_point_id"
                " AND"
                "  path_name.id = trace_point.path_id"
                " AND"
                "  function_name.id = trace_point.function_id" +
                filterClause(m_filter) +
                " ORDER BY"
                "  process.name" );

    if (!result) {
        *errMsg = query.lastError().text();
        return false;
    }

    setUpdatesEnabled( false );

    while ( query.next() ) {
        TreeItem *applicationItem = 0;
        {
            const QString application = QString( "%1 (PID %2)" )
                                            .arg( query.value( 0 ).toString() )
                                            .arg( query.value( 1 ).toString() );
            ItemMap::ConstIterator it = m_applicationItems.find( application );
            if ( it != m_applicationItems.end() ) {
                applicationItem = *it;
            } else {
                applicationItem = new TreeItem( new QTreeWidgetItem( this,
                                                       QStringList() << application ) );
                m_applicationItems[ application ] = applicationItem;
            }
        }

        TreeItem *sourceFileItem = 0;
        {
            const QString sourceFile = query.value( 2 ).toString();
            ItemMap::ConstIterator it = applicationItem->children.find( sourceFile );
            if ( it != applicationItem->children.end() ) {
                sourceFileItem = *it;
            } else {
                sourceFileItem = new TreeItem( new QTreeWidgetItem( applicationItem->item,
                                                      QStringList() << sourceFile ) );
                applicationItem->children[ sourceFile ] = sourceFileItem;
            }
        }

        TreeItem *functionItem = 0;
        {
            const QString function = QString( "%1 (line %2)" )
                                        .arg( query.value( 4 ).toString() )
                                        .arg( query.value( 3 ).toString() );
            ItemMap::ConstIterator it = sourceFileItem->children.find( function );
            if ( it != sourceFileItem->children.end() ) {
                functionItem = *it;
            } else {
                functionItem = new TreeItem( new QTreeWidgetItem( sourceFileItem->item,
                                                    QStringList() << function ) );
                sourceFileItem->children[ function ] = functionItem;
            }

        }

        TreeItem *variableItem = 0;
        {
            using TRACELIB_NAMESPACE_IDENT(VariableType);

            const VariableType::Value varType = static_cast<VariableType::Value>( query.value( 6 ).toInt() );
            const QString varName = QString( "%1 (%2)" )
                                        .arg( query.value( 5 ).toString() )
                                        .arg( VariableType::valueAsString( varType ) );
            ItemMap::ConstIterator it = functionItem->children.find( varName );
            if ( it != functionItem->children.end() ) {
                variableItem = *it;
            } else {
                variableItem = new TreeItem( new QTreeWidgetItem( functionItem->item,
                                                    QStringList() << QString() << varName ) );
                functionItem->children[ varName ] = variableItem;
            }
        }

        const QString currentValue = variableItem->item->data( 2, Qt::DisplayRole ).toString();
        const QString varValue = query.value( 7 ).toString();
        if ( currentValue != varValue ) {
            variableItem->item->setData( 3, Qt::DisplayRole, currentValue );
            variableItem->item->setData( 3, Qt::ToolTipRole, currentValue );
            variableItem->item->setData( 2, Qt::DisplayRole, varValue );
            variableItem->item->setData( 2, Qt::ToolTipRole, varValue );
        }
    }

    setUpdatesEnabled( true );

    m_dirty = false;

    return true;
}

void WatchTree::reApplyFilter()
{
    m_dirty = true;

    QString errMsg;
    if (!showNewTraceEntries(&errMsg)) {
        qDebug() << "WatchTree::reApplyFilter: failed: " << errMsg;
    }
}
