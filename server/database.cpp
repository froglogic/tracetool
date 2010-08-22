#include "database.h"

#include <cassert>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

const int Database::expectedVersion = 0;

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

