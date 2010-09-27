/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "applicationtable.h"

#include <QHeaderView>

#include <assert.h>

ApplicationTable::ApplicationTable()
    : QTableWidget( 0, 4 )
{
    setAlternatingRowColors( true );
    setSelectionMode( QAbstractItemView::NoSelection );
    setHorizontalHeaderLabels( QStringList()
            << tr( "PID" )
            << tr( "Application" )
            << tr( "Start Time" )
            << tr( "End Time" )
            );
    verticalHeader()->setVisible( false );
}

void ApplicationTable::setApplications( const QList<TracedApplicationInfo> &apps )
{
    setUpdatesEnabled( false );
    setSortingEnabled( false );

    clearContents();
    m_items.clear();

    setRowCount( apps.count() );

    int currentRow = 0;
    QList<TracedApplicationInfo>::ConstIterator it, end = apps.end();
    for ( it = apps.begin(); it != end; ++it, ++currentRow ) {
        TracedApplicationId id;
        id.name = it->name;
        id.startTime = it->startTime;
        assert( !m_items.contains( id ) );

        m_items[id] = insertEntry( currentRow, it->pid, it->name, it->startTime, it->stopTime );
    }

    setSortingEnabled( true );
    setUpdatesEnabled( true );
}

void ApplicationTable::handleNewTraceEntry( const TraceEntry &entry )
{
    TracedApplicationId id;
    id.name = entry.processName;
    id.startTime = entry.processStartTime;

    QMap<TracedApplicationId, QTableWidgetItem *>::Iterator it = m_items.find( id );
    if ( it == m_items.end() ) {
        const int rows = rowCount();
        setRowCount( rows + 1 );
        m_items[id] = insertEntry( rows, entry.pid, entry.processName, entry.processStartTime, QDateTime() );
    } else {
        setTimesForApplication( *it, entry.processStartTime, QDateTime() );
    }
}

void ApplicationTable::handleProcessShutdown( const ProcessShutdownEvent &ev )
{
    TracedApplicationId id;
    id.name = ev.name;
    id.startTime = ev.startTime;

    QMap<TracedApplicationId, QTableWidgetItem *>::Iterator it = m_items.find( id );
    if ( it == m_items.end() ) {
        const int rows = rowCount();
        setRowCount( rows + 1 );
        m_items[id] = insertEntry( rows, ev.pid, ev.name, ev.startTime, ev.stopTime );
    } else {
        setTimesForApplication( *it, QDateTime(), ev.stopTime );
    }
}


QTableWidgetItem *ApplicationTable::insertEntry( int row, unsigned int pid, const QString &name, const QDateTime &startTime, const QDateTime &endTime )
{
    setItem( row, 0, new QTableWidgetItem( QString::number( pid ) ) );

    QTableWidgetItem *nameItem = new QTableWidgetItem( name );
    setItem( row, 1, nameItem );

    setTimesForApplication( nameItem, startTime, endTime );

    return nameItem;
}

void ApplicationTable::setTimesForApplication( QTableWidgetItem *nameItem,
                                               const QDateTime &startTime,
                                               const QDateTime &endTime )
{
    const int row = nameItem->row();
    if ( !startTime.isNull() ) {
        setItem( row, 2, new QTableWidgetItem( startTime.toString() ) );
    }
    if ( !endTime.isNull() ) {
        setItem( row, 3, new QTableWidgetItem( endTime.toString() ) );
    }
}

