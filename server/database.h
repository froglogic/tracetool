#ifndef DATABASE_H
#define DATABASE_H

class QSqlDatabase;
class QString;

class Database
{
public:
    static const int expectedVersion;
    static int currentVersion( QSqlDatabase db, QString *errMsg );
    static bool checkCompatibility( QSqlDatabase db, QString *detail );
};

#endif
