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
    if (!matchesAnything())
        return false;
    // Check is analog to LIKE %..% clause in model using a SQL query
    if (!m_application.isEmpty() && !e.processName.contains(m_application))
        return false;
    if (m_processId != -1 && m_processId != e.pid)
        return false;
    if (m_threadId != -1 && m_threadId != e.tid)
        return false;
    if (m_function.isEmpty() && !e.function.contains(m_function))
        return false;
    if (m_message.isEmpty() && !e.message.contains(m_message))
        return false;
    if (m_type != -1 && m_type != e.type)
        return false;
    if (e.groupName.isNull() && !m_acceptsEntriesWithoutKey)
        return false;
    if (!m_acceptableKeys.contains(e.groupName))
        return false;
    return true;
}

bool EntryFilter::matchesAnything() const
{
    /* If we don't accept entries without keys, but we also don't
     * list any keys which we would accept, then we will never
     * match anything.
     */
    if (!m_acceptsEntriesWithoutKey && m_acceptableKeys.isEmpty())
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
    {
        QStringList groupIdTests;
        if (m_acceptsEntriesWithoutKey)
            groupIdTests.append("trace_point.group_id = 0");
        if (!m_acceptableKeys.isEmpty())
            groupIdTests.append("trace_point.group_id = trace_point_group.id");
        if (!groupIdTests.isEmpty())
            expressions.append(QString("(%1)").arg(groupIdTests.join(" OR ")));
    }

    {
        QStringList keyTests;
        QStringList::ConstIterator it, end = m_acceptableKeys.end();
        for (it = m_acceptableKeys.begin(); it != end; ++it)
            keyTests.append(QString("trace_point_group.name = '%1'").arg(*it));
        if (!keyTests.isEmpty())
            expressions.append(QString("(%1)").arg(keyTests.join(" OR ")));
    }
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

