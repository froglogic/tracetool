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

#ifndef APPLICATIONTABLE_H
#define APPLICATIONTABLE_H

#include "../server/database.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QString>
#include <QTableWidget>

QString formatDateTimeForDisplay( const QDateTime & );

class ApplicationTable : public QTableWidget
{
    Q_OBJECT
public:
    ApplicationTable();

    void setApplications( const QList<TracedApplicationInfo> &apps );

public slots:
    void handleNewTraceEntry( const TraceEntry &entry );
    void handleProcessShutdown( const ProcessShutdownEvent &ev );

private:
    struct TracedApplicationId {
        unsigned int pid;
        QString name;
        QDateTime startTime;
        bool operator<( const TracedApplicationId &other ) const {
            if ( name < other.name )
                return true;
            if ( name > other.name )
                return false;
            if ( startTime < other.startTime )
                return true;
            return startTime == other.startTime && pid < other.pid;
        }
    };

    QTableWidgetItem *insertEntry(int row,
            unsigned int pid,
            const QString &name,
            const QDateTime &startTime,
            const QDateTime &endTime );
    void setTimesForApplication(QTableWidgetItem *nameItem,
                                const QDateTime &startTime,
                                const QDateTime &endTime);

    QMap<TracedApplicationId, QTableWidgetItem *> m_items;
};

#endif // !defined(APPLICATIONTABLE_H)
