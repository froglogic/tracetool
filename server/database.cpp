/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "database.h"

#include <cassert>
#include <stdexcept>
#include <QDebug>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

// just for convenience and encoding safety
class Qruntime_error : public std::runtime_error
{
public:
    Qruntime_error(const QString &what)
        : runtime_error(what.toUtf8().constData()) { }
};

Transaction::Transaction( QSqlDatabase db )
    : m_query( db ),
    m_commitChanges( true )
{
    m_query.setForwardOnly( true );
    m_query.exec( "BEGIN TRANSACTION;" );
}

Transaction::~Transaction()
{
    m_query.exec( m_commitChanges ? "COMMIT;" : "ROLLBACK;" );
}

QVariant Transaction::exec( const QString &statement )
{
    if ( !m_query.exec( statement ) ) {
        m_commitChanges = false;
        throw SQLTransactionException( QString( "Failed to store entry in database: executing SQL command '%1' failed: %2" )
                                        .arg( statement ).arg( m_query.lastError().text() ),
                                       m_query.lastError().text(),
                                       m_query.lastError().number() );
    }
    if ( m_query.next() ) {
        return m_query.value( 0 );
    }
    return QVariant();
}

const int Database::expectedVersion = 4;

static const char * const schemaStatements[] = {
    "CREATE TABLE schema_downgrade (from_version INTEGER,"
    " statements TEXT);",
    "CREATE TABLE trace_entry (id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " traced_thread_id INTEGER,"
    " timestamp DATETIME,"
    " trace_point_id INTEGER,"
    " message TEXT,"
    " stack_position INTEGER);",
    "CREATE TABLE trace_point (id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " type INTEGER,"
    " path_id INTEGER,"
    " line INTEGER,"
    " function_id INTEGER,"
    " group_id INTEGER,"
    " UNIQUE(type, path_id, line, function_id, group_id));",
    "CREATE TABLE function_name (id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " name TEXT,"
    " UNIQUE(name));",
    "CREATE TABLE path_name (id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " name TEXT,"
    " UNIQUE(name));",
    "CREATE TABLE process (id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " name TEXT,"
    " pid INTEGER,"
    " start_time DATETIME,"
    " end_time DATETIME,"
    " UNIQUE(name, pid));",
    "CREATE TABLE traced_thread (id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " process_id INTEGER,"
    " tid INTEGER,"
    " UNIQUE(process_id, tid));",
    "CREATE TABLE variable (trace_entry_id INTEGER,"
    " name TEXT,"
    " value TEXT,"
    " type INTEGER);",
    "CREATE TABLE stackframe (trace_entry_id INTEGER,"
    " depth INTEGER,"
    " module_name TEXT,"
    " function_name TEXT,"
    " offset INTEGER,"
    " file_name TEXT,"
    " line INTEGER);",
    "CREATE TABLE trace_point_group(id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " name TEXT,"
    " UNIQUE(name));"
};

static const char * const downgradeStatementsInsert[] = {
    0, // can't downgrade further than version 0
    "INSERT INTO schema_downgrade VALUES(1, 'NOT IMPLEMENTED');",
    "INSERT INTO schema_downgrade VALUES(2, 'NOT IMPLEMENTED');",
    "INSERT INTO schema_downgrade VALUES(3, 'NOT IMPLEMENTED');",
    "INSERT INTO schema_downgrade VALUES(4, 'NOT IMPLEMENTED');"
};

int Database::currentVersion( QSqlDatabase db, QString *errMsg )
{
    assert( errMsg != NULL );
    QSqlQuery query( db );
    if ( !query.exec( "SELECT COUNT(*) FROM schema_downgrade;" ) ) {
        *errMsg = query.lastError().text();
        return -1;
    }
    if ( !query.next() ) {
        *errMsg = query.lastError().text();
        return -1;
    }
    QVariant resultValue = query.value( 0 );
    assert( resultValue.isValid() );
    bool ok;
    int ver = resultValue.toInt( &ok );
    assert( ok );
    return ver;
}

bool Database::checkCompatibility( QSqlDatabase db, QString *errMsg )
{
    assert( errMsg != NULL );
    int current = currentVersion( db, errMsg );
    if ( current == -1 )
        return false;
    
    if ( current == expectedVersion )
        return true;

    *errMsg = QObject::tr( "Database version mismatch. "
                           "Expected %1, found %2.\n"
                           "You can run a conversion tool to upgrade and "
                           "downgrade the database, respectively." )
        .arg( current ).arg( expectedVersion );

    return false;
}

// includes version check
QSqlDatabase Database::open(const QString &fileName,
			    QString *errMsg)
{
    QSqlDatabase db = openAnyVersion(fileName, errMsg);
    if (!db.isValid())
	return QSqlDatabase();
    if (!checkCompatibility(db, errMsg))
	return QSqlDatabase();
    return db;
}

QSqlDatabase Database::create(const QString &fileName,
			      QString *errMsg)
{
    // At least with Sqlite this will also create a
    // new database
    QSqlDatabase db = openOrCreate(fileName, errMsg);
    if (!db.isValid())
	return db;

    QSqlQuery query(db);
    query.exec("BEGIN TRANSACTION;");
    for (unsigned i = 0; i < sizeof(schemaStatements) / sizeof(schemaStatements[0]); ++i) {
	if (!query.exec(schemaStatements[i])) {
	    *errMsg = QObject::tr("Failed to execute '%1': %2")
		.arg(schemaStatements[i])
		.arg(query.lastError().text());
	    query.exec("ROLLBACK;");
	    return QSqlDatabase();
	}
    }
    // statements that allow users of older versions
    // to downgrade a database created by us
    for (int v = 1; v <= expectedVersion; ++v) {
	if (!query.exec(downgradeStatementsInsert[v])) {
	    *errMsg = QObject::tr("Failed to execute '%1': %2")
		.arg(downgradeStatementsInsert[v])
		.arg(query.lastError().text());
	    query.exec("ROLLBACK;");
	    return QSqlDatabase();
	}
    }

    query.exec("COMMIT;");

    return db;
}

QSqlDatabase Database::openOrCreate(const QString &fileName,
				    QString *errMsg)
{
    const QString driverName = "QSQLITE";
    if (!QSqlDatabase::isDriverAvailable(driverName)) {
        *errMsg = QObject::tr("Missing required %1 driver.")
	    .arg(driverName);
        return QSqlDatabase();
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(driverName,
						fileName);
    db.setDatabaseName(fileName);
    if (!db.open()) {
        *errMsg = db.lastError().text();
        return QSqlDatabase();
    }

    return db;
}

QSqlDatabase Database::openAnyVersion(const QString &fileName,
				      QString *errMsg)
{
    if (!QFile::exists(fileName)) {
	*errMsg = QObject::tr("Database %1 not found").arg(fileName);
	return QSqlDatabase();
    }
    return openOrCreate(fileName, errMsg);
}

static QString downgradeStatementsForVersion(QSqlDatabase db,
                                             int version)
{
    QSqlQuery query(db);
    const QString sql = QString("SELECT statements FROM schema_downgrade "
                                "WHERE from_version = %1").arg(version);
    if (!query.exec(sql)) {
        throw Qruntime_error(query.lastError().text());
    }
    if (!query.next()) {
        QString errMsg = QString("Did not find SQL statements for downgrading "
                                 "from version %1.").arg(version);
        throw Qruntime_error(errMsg);
    }

    return query.value(0).toString();
}

static void downgradeVersion(QSqlDatabase db, int version)
{
    QString sql = downgradeStatementsForVersion(db, version);
    db.transaction();
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        db.rollback();
        throw Qruntime_error(query.lastError().text());
    }
    // even remove the downgrade statements to make the conversion
    // perfect. remember that they are being used to designate the
    // current version number.
    QString del = QString("DELETE FROM schema_downgrade "
                          "WHERE from_version = %1").arg(version);
    if (!query.exec(del)) {
        db.rollback();
        throw Qruntime_error(query.lastError().text());
    }
    db.commit();
}

bool Database::downgrade(QSqlDatabase db, QString *errMsg)
{
    const int current = currentVersion(db, errMsg);
    if (current == -1)
	return false;
    if (current == expectedVersion) {
	*errMsg = QObject::tr("Database already of right version.");
	return false;
    }
    if (current < expectedVersion) {
	*errMsg = QObject::tr("Database older than ours.");
	return false;
    }

    for (int v = current; v > expectedVersion; --v) {
        try {
            downgradeVersion(db, v);
        } catch (const std::exception &e) {
            *errMsg = QString::fromUtf8(e.what());
            return false;
        }
    }
    return true;
}

static bool upgradeToVersion1(QSqlDatabase db, QString *errMsg)
{
    // Ugly workaround for lack of ADD COLUMN support in Sqlite
    const char* const statements[] = {
	"BEGIN TRANSACTION;",
	"CREATE TEMPORARY TABLE process_backup (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, pid INTEGER, start_time DATETIME, end_time DATETIME);",
	"INSERT INTO process_backup SELECT id, name, pid, NULL, NULL FROM process;",
	"DROP TABLE process;",
	"CREATE TABLE process (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, pid INTEGER, start_time DATETIME, end_time DATETIME, UNIQUE(name, pid));",
	"INSERT INTO process SELECT id, name, pid, start_time, end_time FROM process_backup;",
	"DROP TABLE process_backup;",
	downgradeStatementsInsert[1],
	"COMMIT;" };
    QSqlQuery query(db);
    for (unsigned i = 0; i < sizeof(statements)/sizeof(char*); ++i) {
	if (!query.exec(statements[i])) {
	    *errMsg = query.lastError().text();
	    return false;
	}
    }
    return true;
}

static bool upgradeVersion(QSqlDatabase db, int version,
			   QString *errMsg)
{
    switch (version) {
    case 0:
	return upgradeToVersion1(db, errMsg);
	break;
    default:
	*errMsg = QObject::tr("Automatic upgrade to version %1 is not implemented");
	return false;
    }
}

bool Database::upgrade(QSqlDatabase db, QString *errMsg)
{
    const int current = currentVersion(db, errMsg);
    if (current == -1)
	return false;
    if (current == expectedVersion) {
	*errMsg = QObject::tr("Database already up to date");
	return false;
    }
    if (current > expectedVersion) {
	*errMsg = QObject::tr("Database already of higher version.");
	return false;
    }
    for (int v = current; v < expectedVersion; ++v) {
	if (!upgradeVersion(db, v, errMsg))
	    return false;
    }
    return true;
}

bool Database::isValidFileName(const QString &fileName,
                               QString *errMsg)
{
#ifdef Q_OS_WIN
    const Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#endif
    if (!fileName.endsWith(".trace", sensitivity)) {
        *errMsg = QObject::tr("Trace file is expected to have a "
                              ".trace suffix.");
        return false;
    }

    return true;
}

QList<StackFrame> Database::backtraceForEntry(QSqlDatabase db,
                                              unsigned int entryId)
{
    const QString statement = QString(
                      "SELECT"
                      " module_name,"
                      " function_name,"
                      " offset,"
                      " file_name,"
                      " line "
                      "FROM"
                      " stackframe "
                      "WHERE"
                      " trace_entry_id=%1 "
                      "ORDER BY"
                      " depth" ).arg( entryId );

    QSqlQuery q( db );
    q.setForwardOnly( true );
    if ( !q.exec( statement ) ) {
        const QString msg = QString( "Failed to retrieve backtrace for trace entry: executing SQL command '%1' failed: %2" )
                        .arg( statement )
                        .arg( q.lastError().text() );
        throw Qruntime_error( msg );
    }

    QList<StackFrame> frames;
    while ( q.next() ) {
        StackFrame f;
        f.module = q.value( 0 ).toString();
        f.function = q.value( 1 ).toString();
        f.functionOffset = q.value( 2 ).toUInt();
        f.sourceFile = q.value( 3 ).toString();
        f.lineNumber = q.value( 4 ).toUInt();
        frames.append( f );
    }

    return frames;
}

QStringList Database::seenGroupIds(QSqlDatabase db)
{
    const QString statement = QString(
                      "SELECT"
                      " name "
                      "FROM"
                      " trace_point_group;" );

    QSqlQuery q( db );
    q.setForwardOnly( true );
    if ( !q.exec( statement ) ) {
        const QString msg = QString( "Failed to retrieve list of available trace groups: executing SQL command '%1' failed: %2" )
                        .arg( statement )
                        .arg( q.lastError().text() );
        throw Qruntime_error( msg );
    }

    QStringList l;
    while ( q.next() ) {
        l.append( q.value( 0 ).toString() );
    }
    return l;
}

#if 0
void Database::addGroupId(QSqlDatabase db, const QString &id)
{
    Transaction transaction( db );
    if ( !transaction.exec( QString( "SELECT id FROM trace_point_group WHERE name=%1;" ).arg( formatValue( db, id ) ) ).isValid() ) {
        transaction.exec( QString( "INSERT INTO trace_point_group VALUES(NULL, %1);" ).arg( formatValue( db, id ) ) );
    }
}
#endif

void Database::trimTo(QSqlDatabase db, size_t nMostRecent)
{
    /* Special handling in case we want to remove all entries from
     * the database; these simple DELETE FROM statements are
     * recognized by sqlite and they run much faster than those
     * with a WHERE clause.
     */
    if ( nMostRecent == 0 ) {
        Transaction transaction( db );
        transaction.exec( "DELETE FROM trace_entry;" );

        // Resets all AUTOINCREMENT fields in trace_entry to zero
        transaction.exec( "DELETE FROM sqlite_sequence WHERE name='trace_entry';" );

        transaction.exec( "DELETE FROM trace_point;" );
        transaction.exec( "DELETE FROM function_name;" );
        transaction.exec( "DELETE FROM path_name;" );
        transaction.exec( "DELETE FROM process;" );
        transaction.exec( "DELETE FROM traced_thread;" );
        transaction.exec( "DELETE FROM variable;" );
        transaction.exec( "DELETE FROM stackframe;" );
#if 0 // cache for the user's convenenience
        transaction.exec( "DELETE FROM trace_point_group;" );
#endif
        return;
    }
    qWarning() << "Server::trimTo: deleting all but the n most recent trace "
                  "entries not implemented yet!";
}

QList<TracedApplicationInfo> Database::tracedApplications(QSqlDatabase db)
{
    const QString statement = QString(
                      "SELECT"
                      " name,"
                      " pid,"
                      " start_time,"
                      " end_time "
                      "FROM"
                      " process;" );

    QSqlQuery q( db );
    q.setForwardOnly( true );
    if ( !q.exec( statement ) ) {
        const QString msg = QString( "Failed to retrieve list of traced applications: executing SQL command '%1' failed: %2" )
                        .arg( statement )
                        .arg( q.lastError().text() );
        throw Qruntime_error( msg );
    }

    QList<TracedApplicationInfo> l;
    while ( q.next() ) {
        TracedApplicationInfo info;
        bool ok;
        info.pid = q.value( 1 ).toUInt( &ok );
        assert( ok );
        info.startTime = QDateTime::fromString( q.value( 2 ).toString(), Qt::ISODate );
        info.stopTime = QDateTime::fromString( q.value( 3 ).toString(), Qt::ISODate );
        info.name = q.value( 0 ).toString();

        l.append( info );
    }
    return l;
}

QDataStream &operator<<( QDataStream &stream, const TraceEntry &entry )
{
    return stream << (quint32)entry.pid
        << entry.processStartTime
        << entry.processName
        << (quint32)entry.tid
        << entry.timestamp
        << (quint8)entry.type
        << entry.path
        << (quint32)entry.lineno
        << entry.groupName
        << entry.function
        << entry.message
        << entry.variables
        << entry.backtrace
        << (quint64)entry.stackPosition
        << entry.traceKeys;
}

QDataStream &operator>>( QDataStream &stream, TraceEntry &entry )
{
    quint32 pid, tid, lineno;
    quint8 type;
    quint64 stackPosition;

    stream >> pid
        >> entry.processStartTime
        >> entry.processName
        >> tid
        >> entry.timestamp
        >> type
        >> entry.path
        >> lineno
        >> entry.groupName
        >> entry.function
        >> entry.message
        >> entry.variables
        >> entry.backtrace
        >> stackPosition
        >> entry.traceKeys;

    entry.pid = pid;
    entry.tid = tid;
    entry.lineno = lineno;
    entry.type = type;
    entry.stackPosition = stackPosition;

    return stream;
}

QDataStream &operator<<( QDataStream &stream, const ProcessShutdownEvent &ev )
{
    return stream << (quint32)ev.pid
        << ev.startTime
        << ev.stopTime
        << ev.name;
}

QDataStream &operator>>( QDataStream &stream, ProcessShutdownEvent &ev )
{
    quint32 pid;
    stream >> pid
        >> ev.startTime
        >> ev.stopTime
        >> ev.name;
    ev.pid = pid;
    return stream;
}

QDataStream &operator<<( QDataStream &stream, const StackFrame &entry )
{
    return stream << entry.module
        << entry.function
        << (quint64)entry.functionOffset
        << entry.sourceFile
        << (quint32)entry.lineNumber;
}

QDataStream &operator>>( QDataStream &stream, StackFrame &entry )
{
    quint64 functionOffset;
    quint32 lineNumber;
    stream >> entry.module
        >> entry.function
        >> functionOffset
        >> entry.sourceFile
        >> lineNumber;
    entry.functionOffset = functionOffset;
    entry.lineNumber = lineNumber;
    return stream;
}

QDataStream &operator<<( QDataStream &stream, const Variable &entry )
{
    return stream << entry.name
        << (quint8)entry.type
        << entry.value;
}

QDataStream &operator>>( QDataStream &stream, Variable &entry )
{
    quint8 type;
    stream >> entry.name
        >> type
        >> entry.value;
    entry.type = (TRACELIB_NAMESPACE_IDENT(VariableType)::Value)type;
    return stream;
}

QDataStream &operator<<( QDataStream &stream, const TraceKey &key )
{
    return stream << key.name << (quint8)( key.enabled ? 1 : 0 );
}

QDataStream &operator>>( QDataStream &stream, TraceKey &key )
{
    quint8 flag;
    stream >> key.name >> flag;
    key.enabled = flag != 0;
    return stream;
}

