#include "database.h"

#include <cassert>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

const int Database::expectedVersion = 1;

int Database::currentVersion( QSqlDatabase db, QString *errMsg )
{
    assert( errMsg != NULL );
    QSqlQuery query( "SELECT COUNT(*) FROM schema_downgrade;", db );
    if ( !query.exec() ) {
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

QSqlDatabase Database::openAnyVersion(const QString &fileName,
				      QString *errMsg)
{
    qDebug("openAnyVersion()");
    if (!QFile::exists(fileName)) {
	*errMsg = QObject::tr("Database %1 not found").arg(fileName);
	return QSqlDatabase();
    }

    const QString driverName = "QSQLITE";
    if (!QSqlDatabase::isDriverAvailable(driverName)) {
        *errMsg = QObject::tr("Missing required %1 driver.")
	    .arg(driverName);
        return QSqlDatabase();
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(driverName,
						"xxx");
    db.setDatabaseName(fileName);
    if (!db.open()) {
        *errMsg = db.lastError().text();
        return QSqlDatabase();
    }

    return db;
}

bool Database::downgrade(QSqlDatabase db, QString *errMsg)
{
    // ###
    return false;
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
}

