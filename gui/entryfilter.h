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

#ifndef ENTRYFILTER_H
#define ENTRYFILTER_H

#include "restorableobject.h"

#include <QObject>
#include <QStringList>

struct TraceEntry;

class EntryFilter : public QObject,
                    public RestorableObject
{
    Q_OBJECT
public:
    EntryFilter(QObject *parent = 0) :
        QObject(parent), m_processId(-1), m_threadId(-1), m_type(-1),
        m_acceptsEntriesWithoutKey(true) { }

    QString application() const { return m_application; }
    void setApplication(const QString &app) { m_application = app; }

    int processId() const { return m_processId; }
    void setProcessId(int pid) { m_processId = pid; }

    int threadId() const { return m_threadId; }
    void setThreadId(int tid) { m_threadId = tid; }

    QString function() const { return m_function; }
    void setFunction(const QString &func) { m_function = func; }

    QString message() const { return m_message; }
    void setMessage(const QString &msg) { m_message = msg; }

    void setInactiveKeys( const QStringList &inactiveKeys ) { m_inactiveKeys = inactiveKeys; }
    QStringList inactiveKeys() const { return m_inactiveKeys; }

    void setAcceptEntriesWithoutKey( bool b ) { m_acceptsEntriesWithoutKey = b; }
    bool acceptsEntriesWithoutKey() const { return m_acceptsEntriesWithoutKey; }

    int type() const { return m_type; }
    void setType(int t) { m_type = t; }

    bool matches(const TraceEntry &e) const;

    // for WHERE clauses in SQL queries
    QString whereClause(const QString &appField,
                        const QString &pidField,
                        const QString &tidField,
                        const QString &funcField,
                        const QString &msgField,
                        const QString &typeField) const;

    // from RestorableObject interface
    QVariant sessionState() const;
    bool restoreSessionState(const QVariant &state);

    // ### Instead of this manual call consider firing event on each
    // ### set*() call. Will likely require some laziness mechanism on
    // ### the receiver side to reduce the number of update.
    void emitChanged() { emit changed(); }

signals:
    void changed();

private:
    QString m_application;
    int m_processId;
    int m_threadId;
    QString m_function;
    QString m_message;
    int m_type;
    QStringList m_inactiveKeys;
    bool m_acceptsEntriesWithoutKey;
};

#endif
