/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "entryfilter.h"

#include "../server/server.h"

#include <QStringList>

typedef QMap<QString, QVariant> StorageMap;

bool EntryFilter::matches(const TraceEntry &e) const
{
    // Check is analog to LIKE %..% clause in model using a SQL query
    if (!m_application.isEmpty() && !e.processName.contains(m_application))
        return false;
    return true;
}

// ### take care of escaping
// ### one day, consider different comparison type
QString EntryFilter::whereClause(const QString &appField,
                                 const QString &pidField,
                                 const QString &tidField,
                                 const QString &funcField,
                                 const QString &msgField,
                                 const QString &typeField) const
{
    QStringList expressions;
    if (!m_application.isEmpty())
        expressions.append(QString("%1 LIKE '%%2%'")
                           .arg(appField).arg(m_application));
    if (m_processId != -1)
        expressions.append(QString("%1 = %2")
                           .arg(pidField).arg(m_processId));
    if (m_threadId != -1)
        expressions.append(QString("%1 = %2")
                           .arg(tidField).arg(m_threadId));
    if (!m_function.isEmpty())
        expressions.append(QString("%1 LIKE '%%2%'")
                           .arg(funcField).arg(m_function));
    if (!m_message.isEmpty())
        expressions.append(QString("%1 LIKE '%%2%'")
                           .arg(msgField).arg(m_message));
    if (m_type != -1)
        expressions.append(QString("%1 = %2")
                           .arg(typeField).arg(m_type));

    if (expressions.isEmpty())
        return QString();
    return expressions.join(" AND ");
}

QVariant EntryFilter::sessionState() const
{
    StorageMap map;
    if (!m_application.isEmpty())
        map["Application"] = m_application;
    if (m_processId != -1)
        map["ProcessId"] = m_processId;
    if (m_threadId != -1)
        map["ThreadId"] = m_threadId;
    if (!m_function.isEmpty())
        map["Function"] = m_function;
    if (!m_message.isEmpty())
        map["Message"] = m_message;
    if (m_type != -1)
        map["Type"] = m_type;

    return QVariant(map);
}

bool EntryFilter::restoreSessionState(const QVariant &state)
{
    StorageMap map = state.value<StorageMap>();
    bool ok;
    m_application = map["Application"].toString();
    m_processId = map["ProcessId"].toInt(&ok);
    if (!ok)
        m_processId = -1;
    m_threadId = map["ThreadId"].toInt(&ok);
    if (!ok)
        m_threadId = -1;
    m_function = map["Function"].toString();
    m_message = map["Message"].toString();
    m_type = map["Type"].toInt(&ok);
    if (!ok)
        m_type = -1;

    emit changed();

    return true;
}

