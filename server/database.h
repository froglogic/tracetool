/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#include <QDateTime>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>

#include "../hooklib/tracelib.h" // for VariableType

class QSqlDatabase;
class QString;

struct StackFrame
{
    QString module;
    QString function;
    size_t functionOffset;
    QString sourceFile;
    size_t lineNumber;
};

QDataStream &operator<<( QDataStream &stream, const StackFrame &entry );
QDataStream &operator>>( QDataStream &stream, StackFrame &entry );

struct Variable
{
    QString name;
    TRACELIB_NAMESPACE_IDENT(VariableType)::Value type;
    QString value;
};

QDataStream &operator<<( QDataStream &stream, const Variable &entry );
QDataStream &operator>>( QDataStream &stream, Variable &entry );

struct TraceEntry
{
    unsigned int pid;
    QDateTime processStartTime;
    QString processName;
    unsigned int tid;
    QDateTime timestamp;
    unsigned int verbosity;
    unsigned int type;
    QString path;
    unsigned long lineno;
    QString groupName;
    QString function;
    QString message;
    QList<Variable> variables;
    QList<StackFrame> backtrace;
    unsigned long stackPosition;
};

QDataStream &operator<<( QDataStream &stream, const TraceEntry &entry );
QDataStream &operator>>( QDataStream &stream, TraceEntry &entry );

struct ProcessShutdownEvent
{
    unsigned int pid;
    QDateTime startTime;
    QDateTime stopTime;
    QString name;
};

QDataStream &operator<<( QDataStream &stream, const ProcessShutdownEvent &ev );
QDataStream &operator>>( QDataStream &stream, ProcessShutdownEvent &ev );

struct TracedApplicationInfo
{
    unsigned int pid;
    QDateTime startTime;
    QDateTime stopTime;
    QString name;
};

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
