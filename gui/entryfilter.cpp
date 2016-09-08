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

#include "entryfilter.h"

#include "../server/server.h"

#include <QStringList>

typedef QMap<QString, QVariant> StorageMap;

bool EntryFilter::matches(const TraceEntry &e) const
{
    // Check is analog to LIKE %..% clause in model using a SQL query
    if (!m_application.isEmpty() && !e.processName.contains(m_application))
        return false;
    if (m_processId != -1 && m_processId != e.pid)
        return false;
    if (m_threadId != -1 && m_threadId != e.tid)
        return false;
    if (!m_function.isEmpty() && !e.function.contains(m_function))
        return false;
    if (!m_message.isEmpty() && !e.message.contains(m_message))
        return false;
    if (m_type != -1 && m_type != e.type)
        return false;
    if (e.groupName.isNull() && !m_acceptsEntriesWithoutKey)
        return false;
    if (m_inactiveKeys.contains(e.groupName))
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
        if (m_acceptsEntriesWithoutKey && !m_inactiveKeys.isEmpty())
            groupIdTests.append("trace_point.group_id = 0");
        if (!m_inactiveKeys.isEmpty()) {
            QStringList groupNameTests;
            groupNameTests.append("trace_point.group_id = trace_point_group.id");
            QStringList::ConstIterator it, end = m_inactiveKeys.end();
            for (it = m_inactiveKeys.begin(); it != end; ++it)
                groupNameTests.append(QString("trace_point_group.name != '%1'").arg(*it));
            groupIdTests.append(QString("(%1)").arg(groupNameTests.join(" AND ")));
        }
        if (!groupIdTests.isEmpty())
            expressions.append(QString("(%1)").arg(groupIdTests.join(" OR ")));
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

