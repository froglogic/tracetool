/**********************************************************************
** Copyright ( C ) 2013 froglogic GmbH.
** All rights reserved.
**********************************************************************/
#include "databasefeeder.h"

#include "database.h"
#include "lru_cache.h"

#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <cassert>
#include <stdexcept>

using namespace std;

static bool getGroupId( QSqlDatabase db, Transaction *transaction, const QString &name, unsigned int *id )
{
    QVariant v = transaction->exec( QString( "SELECT id FROM trace_point_group WHERE name=%1;" ).arg( Database::formatValue( db, name ) ) );
    if ( !v.isValid() ) {
        v = transaction->insert( QString( "INSERT INTO trace_point_group VALUES(NULL, %1);" ).arg( Database::formatValue( db, name ) ) );
    }

    if ( !id ) {
        return true;
    }

    bool ok;
    *id = v.toUInt( &ok );
    return ok;
}

class TraceKeyCache {
public:
    void update( QSqlDatabase db, Transaction *transaction,
         const QString &groupName,
         const QList<TraceKey> &traceKeys ) {
        QList<TraceKey>::ConstIterator it, end = traceKeys.end();
        for ( it = traceKeys.begin(); it != end; ++it ) {
            if ( m_map.find( (*it).name ) == m_map.end() ) {
                registerGroupName( db, transaction, (*it).name );
            }
        }
        // in case the entry comes with a name not listed in the
        // AUT-side configuration file
        if ( !groupName.isNull() && m_map.find( groupName ) == m_map.end() ) {
            registerGroupName( db, transaction, groupName );
        }
    }
    void clear() {
        m_map.clear();
    }
    unsigned int fetch( const QString &name ) const {
        std::map<QString, unsigned int>::const_iterator it = m_map.find( name );
        if ( it == m_map.end() ) {
            throw runtime_error( QString( "Failed to find trace point group %1 in cache" ).arg( name ).toUtf8().constData() );
        }
        return (*it).second;
    }
private:
    void registerGroupName( QSqlDatabase db, Transaction *transaction, const QString &name )
    {
        unsigned int id;
        if ( !getGroupId( db, transaction, name, &id ) ) {
            throw runtime_error( "Read non-numeric trace point group id from database - corrupt database?" );
        }
        m_map[name] = id;
    }

    std::map<QString, unsigned int> m_map;
} traceKeyCache;

template <typename KeyType, typename IdType, int CacheSize = 10>
class StorageCache
{
public:
    StorageCache() : m_lru( CacheSize ) { }
    void clear()
    {
    m_lru.clear();
    }
protected:
    typedef KeyType CacheKey;

    IdType* checkCache( const KeyType &key )
    {
    return m_lru.fetch_ptr( key );
    }
    void cache( const KeyType &key, IdType id )
    {
    m_lru.insert( key, id );
    }

private:
    LRUCache<KeyType, IdType> m_lru;
};

class PathCache : public StorageCache<QString, unsigned int> {
public:
    unsigned int store( QSqlDatabase db, Transaction *transaction,
            const QString &path )
    {
    unsigned int *cachedId = checkCache( path );
    if ( cachedId )
        return *cachedId;
    QVariant v = transaction->exec( QString( "SELECT id FROM path_name WHERE name=%1;" ).arg( Database::formatValue( db, path ) ) );
    if ( !v.isValid() ) {
        v = transaction->insert( QString( "INSERT INTO path_name VALUES(NULL, %1);" ).arg( Database::formatValue( db, path ) ) );
    }
    bool ok;
    unsigned int pathId = v.toUInt( &ok );
    if ( !ok ) {
        throw runtime_error( "Failed to store entry in database: read non-numeric path id from database - corrupt database?" );
    }
    cache( path, pathId );
    return pathId;
    }
} pathCache;

class FunctionCache : public StorageCache<QString, unsigned int> {
public:
    unsigned int store( QSqlDatabase db, Transaction *transaction,
            const QString &function )
    {
    unsigned int *cachedId = checkCache( function );
    if ( cachedId )
        return *cachedId;
    QVariant v = transaction->exec( QString( "SELECT id FROM function_name WHERE name=%1;" ).arg( Database::formatValue( db, function ) ) );
    if ( !v.isValid() ) {
        v = transaction->insert( QString( "INSERT INTO function_name VALUES(NULL, %1);" ).arg( Database::formatValue( db, function ) ) );
    }
    bool ok;
    unsigned int functionId = v.toUInt( &ok );
    if ( !ok ) {
        throw runtime_error( "Failed to store entry in database: read non-numeric function id from database - corrupt database?" );
    }
    cache( function, functionId );
    return functionId;
    }
} functionCache;

class ProcessCache : public StorageCache<std::pair<QString, unsigned int>,
                     unsigned int>
{
public:
    unsigned int store( QSqlDatabase db, Transaction *transaction,
            const QString &processName,
            unsigned int pid,
            const QDateTime &processStartTime )
    {
    CacheKey key( processName, pid );
    unsigned int *cachedId = checkCache( key );
    if ( cachedId )
        return *cachedId;
    QVariant v = transaction->exec( QString( "SELECT id FROM process WHERE pid=%1 AND start_time=%2;" ).arg( pid ).arg( Database::formatValue( db, processStartTime ) ) );
    if ( !v.isValid() ) {
        v = transaction->insert( QString( "INSERT INTO process VALUES(NULL, %1, %2, %3, 0);" ).arg( Database::formatValue( db, processName ) ).arg( pid ).arg( Database::formatValue( db, processStartTime ) ) );
    }
    bool ok;
    unsigned int processId = v.toUInt( &ok );
    if ( !ok ) {
        throw runtime_error( "Failed to store entry in database: read non-numeric process id from database - corrupt database?" );
    }
    cache( key, processId );
    return processId;
    }
} processCache;

class ThreadCache : public StorageCache<std::pair<unsigned int, unsigned int>,
                    unsigned int>
{
public:
    unsigned int store( QSqlDatabase db, Transaction *transaction,
            unsigned int processId,
            unsigned int tid )
    {
    CacheKey key( processId, tid );
    unsigned int *cachedId = checkCache( key );
    if ( cachedId )
        return *cachedId;

    QVariant v = transaction->exec( QString( "SELECT id FROM traced_thread WHERE process_id=%1 AND tid=%2;" ).arg( processId ).arg( tid ) );
    if ( !v.isValid() ) {
        v = transaction->insert( QString( "INSERT INTO traced_thread VALUES(NULL, %1, %2);" ).arg( processId ).arg( tid ) );
    }
    bool ok;
    unsigned int threadId = v.toUInt( &ok );
    if ( !ok ) {
        throw runtime_error( "Failed to store entry in database: read non-numeric traced thread id from database - corrupt database?" );
    }
    cache( key, threadId );
    return threadId;
    }
} threadCache;

static unsigned int storeGroup( QSqlDatabase db, Transaction *transaction,
                const QString &groupName,
                const QList<TraceKey> &traceKeys )
{
    traceKeyCache.update( db, transaction, groupName, traceKeys );

    unsigned int groupId = 0;
    if ( !groupName.isNull() ) {
    groupId = traceKeyCache.fetch( groupName );
    }
    return groupId;
}

// ### some portable, ready-made tuple template type would be nice
struct TracePointTuple
{
    unsigned int type;
    unsigned int pathId;
    unsigned long lineno;
    unsigned int functionId;
    unsigned int groupId;

    bool operator<(const TracePointTuple &tp) const
    {
        if ( type != tp.type ) return type < tp.type;
        if ( pathId != tp.pathId ) return pathId < tp.pathId;
        if ( lineno != tp.lineno ) return lineno < tp.lineno;
        if ( functionId != tp.functionId ) return functionId < tp.functionId;
        if ( groupId != tp.groupId ) return groupId < tp.groupId;
        return false;
    };
};

class TracePointCache : public StorageCache<TracePointTuple,
                        unsigned int>
{
public:
    unsigned int store( QSqlDatabase db, Transaction *transaction,
            unsigned int type,
            unsigned int pathId,
            unsigned long lineno,
            unsigned int functionId,
            unsigned int groupId )
    {
    CacheKey key;
    key.type = type;
    key.pathId = pathId;
    key.lineno = lineno;
    key.functionId = functionId;
    key.groupId = groupId;
    unsigned int *cachedId = checkCache( key );
    if ( cachedId )
        return *cachedId;
    QVariant v = transaction->exec( QString( "SELECT id FROM trace_point WHERE type=%1 AND path_id=%2 AND line=%3 AND function_id=%4 AND group_id=%5;" ).arg( type ).arg( pathId ).arg( lineno ).arg( functionId ).arg( groupId ) );
    if ( !v.isValid() ) {
        v = transaction->insert( QString( "INSERT INTO trace_point VALUES(NULL, %1, %2, %3, %4, %5);" ).arg( type ).arg( pathId ).arg( lineno ).arg( functionId ).arg( groupId ) );
    }
    bool ok;
    unsigned int tracepointId = v.toUInt( &ok );
    if ( !ok ) {
        throw runtime_error( "Failed to store entry in database: read non-numeric tracepoint id from database - corrupt database?" );
    }
    cache( key, tracepointId );
    return tracepointId;
    }
} tracePointCache;

static unsigned int storeTraceEntry( QSqlDatabase db, Transaction *transaction,
                     unsigned int threadId,
                     const QDateTime &timestamp,
                     unsigned int pointId,
                     const QString &message,
                     unsigned long stackPosition )
{
    return transaction->insert( QString( "INSERT INTO trace_entry VALUES(NULL, %1, %2, %3, %4, %5)" )
                    .arg( threadId )
                    .arg( Database::formatValue( db, timestamp ) )
                    .arg( pointId )
                    .arg( Database::formatValue( db, message ) )
                    .arg( stackPosition ) ).toUInt();
}

static void storeVariables( QSqlDatabase db, Transaction *transaction,
                unsigned int traceentryId,
                const QList<Variable> &variables )
{
    QList<Variable>::ConstIterator it, end = variables.end();
    for ( it = variables.begin(); it != end; ++it ) {
        transaction->exec( QString( "INSERT INTO variable VALUES(%1, %2, %3, %4);" ).arg( traceentryId ).arg( Database::formatValue( db, it->name ) ).arg( Database::formatValue( db, it->value ) ).arg( it->type ) );
    }
}

static void storeBacktrace( QSqlDatabase db, Transaction *transaction,
                unsigned int traceentryId,
                const QList<StackFrame> &backtrace )

{
    unsigned int depthCount = 0;
    QList<StackFrame>::ConstIterator it, end = backtrace.end();
    for ( it = backtrace.begin(); it != end; ++it, ++depthCount ) {
        transaction->exec( QString( "INSERT INTO stackframe VALUES(%1, %2, %3, %4, %5, %6, %7);" ).arg( traceentryId ).arg( depthCount ).arg( Database::formatValue( db, it->module ) ).arg( Database::formatValue( db, it->function ) ).arg( it->functionOffset ).arg( Database::formatValue( db, it->sourceFile ) ).arg( it->lineNumber ) );
    }
}

static void storeEntry( QSqlDatabase db, Transaction *transaction, const TraceEntry &e )
{
    unsigned int pathId = pathCache.store( db, transaction, e.path );
    unsigned int functionId = functionCache.store( db, transaction, e.function );
    unsigned int processId = processCache.store( db, transaction, e.processName,
                         e.pid, e.processStartTime );
    unsigned int threadId = threadCache.store( db, transaction, processId, e.tid );
    unsigned int groupId = storeGroup( db, transaction,
                       e.groupName,
                       e.traceKeys );
    unsigned int tracepointId = tracePointCache.store( db, transaction,
                               e.type, pathId, e.lineno,
                               functionId, groupId );
    unsigned int traceentryId = storeTraceEntry( db, transaction,
                         threadId,
                         e.timestamp,
                         tracepointId,
                         e.message,
                         e.stackPosition );
    storeVariables( db, transaction, traceentryId, e.variables );
    storeBacktrace( db, transaction, traceentryId, e.backtrace );
}

static QString archiveFileName( const QString &archiveDirName, const QString &currentFileName )
{
    const QDir archiveDir( archiveDirName );

    const QStringList entries = archiveDir.entryList( QStringList()
                                                      << "*-" + QFileInfo( currentFileName ).fileName() );
    return QString( "%1/%2-%3" )
        .arg( archiveDirName )
        .arg( entries.size() + 1 )
        .arg( QFileInfo( currentFileName ).fileName() );
}

static void archiveEntries( QSqlDatabase db, unsigned short percentage, const QString &archiveDir )
{
    if ( percentage == 0 ) {
        return;
    }

    if ( percentage > 100 ) {
        percentage = 100;
    }

    Transaction transaction( db );
    qulonglong numCopy = 0;
    {
        QVariant v = transaction.exec( QString( "SELECT ROUND(COUNT(id) / 100.0 * %1) FROM trace_entry;" ).arg( percentage ) );
        bool ok;
        numCopy = v.toULongLong( &ok );
        if ( !ok ) {
            throw runtime_error( "Failed to count number of entries to archive" );
        }
    }

    QString connName;
    {
        QSqlDatabase archiveDB;
        {
            if ( !QDir().mkpath( archiveDir ) ) {
                throw runtime_error( QString( "Failed to create archive database: creating archive directory %1 failed" ).arg( archiveDir ).toUtf8().constData() );
            }

            const QString fn = archiveFileName( archiveDir, db.databaseName() );
            QString errorMsg;
            archiveDB = Database::create( fn, &errorMsg );
            if ( !archiveDB.isValid() ) {
                throw runtime_error( QString( "Failed to create database in %1: %2" ).arg( fn ).arg( errorMsg ).toUtf8().constData() );
            }
        }
        connName = archiveDB.connectionName();

        {
            QString query = QString( "SELECT"
                            " trace_entry.id,"
                            " process.pid,"
                            " process.start_time,"
                            " process.name,"
                            " traced_thread.tid,"
                            " trace_entry.timestamp,"
                            " trace_point.type,"
                            " path_name.name,"
                            " trace_point.line,"
                            " trace_point.group_id,"
                            " function_name.name,"
                            " trace_entry.message, "
                            " trace_entry.stack_position "
                            "FROM"
                            " trace_entry,"
                            " trace_point,"
                            " path_name,"
                            " function_name,"
                            " process,"
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
                            " traced_thread.process_id = process.id "
                            "ORDER BY"
                            " trace_entry.id "
                            "LIMIT"
                            " %1" ).arg( numCopy );

            QSqlQuery q( db );
            q.setForwardOnly( true );
            if ( !q.exec( query ) ) {
                throw runtime_error( QString( "Cannot archive trace data: failed to extract entry data: %1" ).arg( q.lastError().text() ).toUtf8().constData() );
            }

            Transaction archiveTransaction( archiveDB );
            while ( q.next() ) {
                qulonglong id = q.value( 0 ).toULongLong();

                TraceEntry e;
                e.pid = q.value( 1 ).toUInt();
                e.processStartTime = QDateTime::fromMSecsSinceEpoch( q.value( 2 ).toLongLong() );
                e.processName = q.value( 3 ).toString();
                e.tid = q.value( 4 ).toUInt();
                e.timestamp = QDateTime::fromMSecsSinceEpoch( q.value( 5 ).toLongLong() );
                e.type = q.value( 6 ).toUInt();
                e.path = q.value( 7 ).toString();
                e.lineno = q.value( 8 ).toULongLong();

                const int groupId = q.value( 9 ).toInt();
                if ( groupId != 0 ) {
                    QSqlQuery gq( db );
                    gq.setForwardOnly( true );
                    if ( gq.exec( QString( "SELECT name FROM trace_point_group WHERE id = %1" ).arg( groupId ) ) ) {
                        if ( gq.next() ) {
                            e.groupName = gq.value( 0 ).toString();
                        }
                    }
                }

                e.function = q.value( 10 ).toString();
                e.message = q.value( 11 ).toString();
                e.stackPosition = q.value( 12 ).toULongLong();
                e.backtrace = Database::backtraceForEntry( db, id );

                {
                    QSqlQuery vq( db );
                    vq.setForwardOnly( true );
                    if ( vq.exec( QString( "SELECT "
                                    " name,"
                                    " type,"
                                    " value "
                                    "FROM "
                                    " variable "
                                    "WHERE"
                                    " trace_entry_id = %1" ).arg( id ) ) ) {
                        while ( vq.next() ) {
                            Variable v;
                            v.name = vq.value( 0 ).toString();
                            v.type = (TRACELIB_NAMESPACE_IDENT(VariableType)::Value)( vq.value( 1 ).toInt() );
                            v.value = vq.value( 2 ).toString();
                            e.variables.append( v );
                        }
                    }
                }
                ::storeEntry( archiveDB, &archiveTransaction, e );
            }
        }
    }

    {
        transaction.exec( QString( "DELETE FROM trace_entry WHERE id IN (SELECT id FROM trace_entry ORDER BY id LIMIT %1);" ).arg( numCopy ) );

        transaction.exec( QString( "DELETE FROM trace_point WHERE id NOT IN (SELECT trace_point_id FROM trace_entry);" ) );
        tracePointCache.clear();

        transaction.exec( QString( "DELETE FROM function_name WHERE id NOT IN (SELECT function_id FROM trace_point);" ) );
        functionCache.clear();

        transaction.exec( QString( "DELETE FROM path_name WHERE id NOT IN (SELECT path_id FROM trace_point);" ) );
        pathCache.clear();

        transaction.exec( QString( "DELETE FROM trace_point_group WHERE id NOT IN (SELECT group_id FROM trace_point);" ) );
        traceKeyCache.clear();

        transaction.exec( QString( "DELETE FROM traced_thread WHERE id NOT IN (SELECT traced_thread_id FROM trace_entry);" ) );
        threadCache.clear();

        transaction.exec( QString( "DELETE FROM process WHERE id NOT IN (SELECT process_id FROM traced_thread);" ) );
        processCache.clear();

        transaction.exec( QString( "DELETE FROM variable WHERE trace_entry_id NOT IN (SELECT id FROM trace_entry);" ) );
        transaction.exec( QString( "DELETE FROM stackframe WHERE trace_entry_id NOT IN (SELECT id FROM trace_entry);" ) );
    }
    QSqlDatabase::removeDatabase( connName );
}

DatabaseFeeder::DatabaseFeeder( QSqlDatabase db )
    : m_db( db )
    , m_shrinkBy( 0 )
    , m_maximumSize( StorageConfiguration::UnlimitedTraceSize )
{
    assert( m_db.isValid() );
    m_db.exec( "PRAGMA synchronous=OFF;");
}

void DatabaseFeeder::trimDb()
{
    Database::trimTo( m_db, 0 );
}

// Definition taken from http://www.sqlite.org/c_interface.html
#define SQLITE_FULL        13   /* Insertion failed because database is full */

void DatabaseFeeder::handleTraceEntry( const TraceEntry &e )
{
    try {
        Transaction transaction( m_db );
        ::storeEntry( m_db, &transaction, e );
    } catch ( const SQLTransactionException &ex ) {
        if ( ex.driverCode() == SQLITE_FULL ) {
            archiveEntries( m_db, m_shrinkBy, m_archiveDir );

            archivedEntries();

            handleTraceEntry( e );
        } else {
            throw;
        }
    }
}

void DatabaseFeeder::handleShutdownEvent( const ProcessShutdownEvent &ev )
{
    Transaction transaction( m_db );
    transaction.exec( QString( "UPDATE process SET end_time=%1 WHERE pid=%2 AND start_time=%3;" ).arg( Database::formatValue( m_db, ev.stopTime ) ).arg( ev.pid ).arg( Database::formatValue( m_db, ev.startTime ) ) );
}

template <typename T>
T clamp( T v, T lowerBound, T upperBound ) {
    if ( v < lowerBound ) return lowerBound;
    if ( v > upperBound ) return upperBound;
    return v;
}

void DatabaseFeeder::applyStorageConfiguration( const StorageConfiguration &cfg )
{
    const unsigned short shrinkBy = clamp<unsigned short>( cfg.shrinkBy, 1, 100 );
    if ( m_maximumSize == cfg.maximumSize &&
         m_shrinkBy == shrinkBy &&
         m_archiveDir == cfg.archiveDir ) {
        return;
    }

    if ( cfg.maximumSize == StorageConfiguration::UnlimitedTraceSize ) {
        /* XXX Don't hardcode this default value, might change if sqlite3 was
         * compiled with different settings.
         */
        m_db.exec( "PRAGMA max_page_count=1073741823" );
        m_maximumSize = cfg.maximumSize;
        m_shrinkBy = shrinkBy;
        m_archiveDir = cfg.archiveDir;
        return;
    }

    qulonglong pageSize = 0;
    {
        QSqlQuery q = m_db.exec( "PRAGMA page_size;" );
        if ( !q.next() ) {
            return;
        }

        bool ok;
        pageSize = q.value( 0 ).toULongLong( &ok );
        if ( !ok || pageSize == 0 ) {
            return;
        }
    }

    qulonglong pageCount = 0;
    {
        QSqlQuery q = m_db.exec( "PRAGMA page_count;" );
        if ( !q.next() ) {
            return;
        }

        bool ok;
        pageCount = q.value( 0 ).toULongLong( &ok );
        if ( !ok ) {
            return;
        }
    }

    /* It's possible that the current file is larger than the given
     * maximum size. In that case, lets just use the current size as
     * the maximum to avoid that it grows even further. We cannot shrink
     * existing files, so this is pretty much the best we can do.
     */
    qulonglong maxPageCount = cfg.maximumSize / pageSize;
    if ( pageCount > maxPageCount ) {
        maxPageCount = pageCount;
    }

    m_db.exec( QString( "PRAGMA max_page_count=%1" ).arg( maxPageCount ) );

    m_maximumSize = cfg.maximumSize;
    m_shrinkBy = shrinkBy;
    m_archiveDir = cfg.archiveDir;
}
