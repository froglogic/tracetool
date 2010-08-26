#include "database.h"

#include <cassert>
#include <stdexcept>
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

const int Database::expectedVersion = 1;

static const char * const schemaStatements[] = {
    "CREATE TABLE schema_downgrade (from_version INTEGER,"
    " statements TEXT);",
    "CREATE TABLE trace_entry (id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " traced_thread_id INTEGER,"
    " timestamp DATETIME,"
    " trace_point_id INTEGER,"
    " message TEXT);",
    "CREATE TABLE trace_point (id INTEGER PRIMARY KEY AUTOINCREMENT,"
    " verbosity INTEGER,"
    " type INTEGER,"
    " path_id INTEGER,"
    " line INTEGER,"
    " function_id INTEGER,"
    " UNIQUE(verbosity, type, path_id, line, function_id));",
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
    " line INTEGER);"
};

static const char * const downgradeStatementsInsert[] = {
    0, // can't downgrade further than version 0
    "INSERT INTO schema_downgrade VALUES(1, 'SELECT * FROM FOOBAR');"
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
    for (int i = 0; i < sizeof(schemaStatements) / sizeof(schemaStatements[0]); ++i) {
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
	*errMsg = QObject::tr("Unhandled upgrade step");
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

