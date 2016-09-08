/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <QDateTime>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>

#include "../hooklib/tracelib.h" // for VariableType

#include <stdexcept>

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

struct TraceKey
{
    QString name;
    bool enabled;
};

QDataStream &operator<<( QDataStream &stream, const TraceKey &key );
QDataStream &operator>>( QDataStream &stream, TraceKey &key );

struct TraceEntry
{
    unsigned int pid;
    QDateTime processStartTime;
    QString processName;
    unsigned int tid;
    QDateTime timestamp;
    unsigned int type;
    QString path;
    unsigned long lineno;
    QString groupName;
    QString function;
    QString message;
    QList<Variable> variables;
    QList<StackFrame> backtrace;
    unsigned long stackPosition;
    QList<TraceKey> traceKeys;
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

class SQLTransactionException : public std::runtime_error
{
public:
    SQLTransactionException( const QString &what, const QString &msg, int code )
        : std::runtime_error( what.toUtf8().constData() ),
        m_msg( msg ),
        m_code( code )
    {
    }
    ~SQLTransactionException() throw() { }

    const QString &driverMessage() const { return m_msg; }
    int driverCode() const { return m_code; }

private:
    QString m_msg;
    int m_code;
};

class Transaction
{
public:
    Transaction( QSqlDatabase db );
    ~Transaction();

    QVariant exec( const QString &statement );
    QVariant insert( const QString &statement );

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
#if 0
    static void addGroupId(QSqlDatabase db, const QString &id);
#endif
    static void trimTo(QSqlDatabase db, size_t nMostRecent);
    static QList<TracedApplicationInfo> tracedApplications(QSqlDatabase db);

    // Special cased since QSql* will loose the milliseconds of a QDateTime value
    static inline QString formatValue(QSqlDatabase db, const QDateTime &v)
    {
        qint64 msecs = v.toMSecsSinceEpoch();
        const QVariant variant = QVariant::fromValue(msecs);
        QSqlField field(QString(), variant.type());
        field.setValue(variant);
        return db.driver()->formatValue(field);
    }

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
