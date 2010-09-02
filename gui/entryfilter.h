/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef ENTRYFILTER_H
#define ENTRYFILTER_H

#include "restorableobject.h"

#include <QString>

class EntryFilter : public RestorableObject
{
public:
    EntryFilter() : m_type(-1) { }

    QString application() const { return m_application; }
    void setApplication(const QString &app) { m_application = app; }

    QString processId() const { return m_processId; }
    void setProcessId(const QString &pid) { m_processId = pid; }

    QString threadId() const { return m_threadId; }
    void setThreadId(const QString &tid) { m_threadId = tid; }

    QString function() const { return m_function; }
    void setFunction(const QString &func) { m_function = func; }

    QString message() const { return m_message; }
    void setMessage(const QString &msg) { m_message = msg; }

    int type() const { return m_type; }
    void setType(int t) { m_type = t; }

    QVariant sessionState() const;
    bool restoreSessionState(const QVariant &state);

private:
    QString m_application;
    QString m_processId;
    QString m_threadId;
    QString m_function;
    QString m_message;
    int m_type;
};

#endif
