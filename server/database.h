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

    static QSqlDatabase openAnyVersion(const QString &fileName,
				       QString *errMsg);

    static bool downgrade(QSqlDatabase db, QString *errMsg);
    static bool upgrade(QSqlDatabase db, QString *errMsg);
};

#endif
