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

#include "applicationtable.h"

#include <QHeaderView>

#include <assert.h>

QString formatDateTimeForDisplay(const QDateTime &dt )
{
    return dt.toString( "yyyy-MM-dd hh:mm:ss.zzz" );
}

ApplicationTable::ApplicationTable()
    : QTableWidget( 0, 4 )
{
    setAlternatingRowColors( true );
    setSelectionMode( QAbstractItemView::NoSelection );
    setHorizontalHeaderLabels( QStringList()
            << tr( "Start Time" )
            << tr( "End Time" )
            << tr( "Application" )
            << tr( "PID" )
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
        id.pid = it->pid;
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
    setItem( row, 3, new QTableWidgetItem( QString::number( pid ) ) );

    QTableWidgetItem *nameItem = new QTableWidgetItem( name );
    setItem( row, 2, nameItem );

    setTimesForApplication( nameItem, startTime, endTime );

    return nameItem;
}

void ApplicationTable::setTimesForApplication( QTableWidgetItem *nameItem,
                                               const QDateTime &startTime,
                                               const QDateTime &endTime )
{
    const int row = nameItem->row();
    if ( !startTime.isNull() ) {
        setItem( row, 0, new QTableWidgetItem( formatDateTimeForDisplay( startTime ) ) );
    }
    if ( !endTime.isNull() ) {
        setItem( row, 1, new QTableWidgetItem( formatDateTimeForDisplay( endTime ) ) );
    }
}

