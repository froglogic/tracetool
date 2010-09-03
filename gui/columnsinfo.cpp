/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "columnsinfo.h"

#include "../server/server.h"

#include <QStringList>

typedef QMap<QString, QVariant> StorageMap;

ColumnsInfo::ColumnsInfo(QObject *parent)
{
    m_visible.resize( LastColumnName );
    m_visible[ int(Time) ] = true;
}

bool ColumnsInfo::isVisible( ColumnName cname ) const
{
    return m_visible[ int( cname ) ];
}

QVariant ColumnsInfo::sessionState() const
{
    StorageMap map;
    map[ "TimeVisible" ] = isVisible( Time );

    return QVariant(map);
}

bool ColumnsInfo::restoreSessionState(const QVariant &state)
{
    StorageMap map = state.value<StorageMap>();
    m_visible[ Time ] = map["TimeVisible"].toBool();

    emit changed();

    return true;
}

