/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#include "server.h" // for TracedApplicationInfo etc.

#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>

class QSqlDatabase;
class QString;

class Transaction
{
public:
    Transaction( QSqlDatabase db );
    ~Transaction();

    QVariant exec( const QString &statement );

private:
    Transaction( const Transaction &other );
    void operator=( const Transaction &rhs );

    QSqlQuery m_query;
    bool m_commitChanges;
};

class Database
{
public:
    static const int expectedVersion;
    static int currentVersion( QSqlDatabase db, QString *errMsg );
    static bool checkCompatibility( QSqlDatabase db, QString *detail );

    static QSqlDatabase open(const QString &fileName,
                             QString *errMsg);
    static QSqlDatabase create(const QString &fileName,
                               QString *errMsg);
    static QSqlDatabase openAnyVersion(const QString &fileName,
				       QString *errMsg);

    static bool downgrade(QSqlDatabase db, QString *errMsg);
    static bool upgrade(QSqlDatabase db, QString *errMsg);

    static bool isValidFileName(const QString &fileName,
                                QString *errMsg);

    static QList<StackFrame> backtraceForEntry(QSqlDatabase db,
                                               unsigned int entryId);
    static QStringList seenGroupIds(QSqlDatabase db);
    static void addGroupId(QSqlDatabase db, const QString &id);
    static void trimTo(QSqlDatabase db, size_t nMostRecent);
    static QList<TracedApplicationInfo> tracedApplications(QSqlDatabase db);

    template <typename T>
    static inline QString formatValue(QSqlDatabase db, const T &v )
    {
        const QVariant variant = QVariant::fromValue(v);
        QSqlField field(QString(), variant.type());
        field.setValue(variant);
        return db.driver()->formatValue(field);
    }

private:
    static QSqlDatabase openOrCreate(const QString &fileName,
                                     QString *errMsg);
};

#endif
