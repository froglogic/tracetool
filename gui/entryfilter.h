/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef ENTRYFILTER_H
#define ENTRYFILTER_H

#include "restorableobject.h"

#include <QObject>
#include <QString>

struct TraceEntry;

class EntryFilter : public QObject,
                    public RestorableObject
{
    Q_OBJECT
public:
    EntryFilter(QObject *parent = 0) :
        QObject(parent), m_processId(-1), m_threadId(-1), m_type(-1) { }

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
};

#endif
