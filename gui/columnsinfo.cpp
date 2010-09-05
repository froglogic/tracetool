/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "columnsinfo.h"

#include <QStringList>

typedef QMap<QString, QVariant> StorageMap;

ColumnsInfo::ColumnsInfo(QObject *parent)
    : m_visible(LastColumnName, true)
{
}

bool ColumnsInfo::isVisible( ColumnName cname ) const
{
    return m_visible[ int( cname ) ];
}

void ColumnsInfo::setVisible( ColumnName cname, bool visible )
{
    m_visible[ int( cname ) ] = visible;
}

QVariant ColumnsInfo::sessionState() const
{
    StorageMap map;
    map[ "TimeVisible" ] = isVisible( Time );
    map[ "ApplicationVisible" ] = isVisible( Application );
    map[ "PIDVisible" ] = isVisible( PID );
    map[ "ThreadVisible" ] = isVisible( Thread );
    map[ "FileVisible" ] = isVisible( File );
    map[ "LineVisible" ] = isVisible( Line );
    map[ "FunctionVisible" ] = isVisible( Function );
    map[ "TypeVisible" ] = isVisible( Type );
    map[ "MessageVisible" ] = isVisible( Message );
    map[ "StackPositionVisible" ] = isVisible( StackPosition );

    return QVariant(map);
}

bool ColumnsInfo::restoreSessionState(const QVariant &state)
{
    StorageMap map = state.value<StorageMap>();
    m_visible[ Time ] = map["TimeVisible"].toBool();
    m_visible[ Application ] = map["ApplicationVisible"].toBool();
    m_visible[ PID ] = map["PIDVisible"].toBool();
    m_visible[ Thread ] = map["ThreadVisible"].toBool();
    m_visible[ File ] = map["FileVisible"].toBool();
    m_visible[ Line ] = map["LineVisible"].toBool();
    m_visible[ Function ] = map["FunctionVisible"].toBool();
    m_visible[ Type ] = map["TypeVisible"].toBool();
    m_visible[ Message ] = map["MessageVisible"].toBool();
    m_visible[ StackPosition ] = map["StackPositionVisible"].toBool();

    emit changed();

    return true;
}

