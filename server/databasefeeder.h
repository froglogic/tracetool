/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2013-2016 froglogic GmbH
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

#ifndef TRACER_DATABASEFEEDER_H
#define TRACER_DATABASEFEEDER_H

#include "xmlcontenthandler.h"

class DatabaseFeeder : public XmlParseEventsHandler
{
public:
    DatabaseFeeder( QSqlDatabase db );
protected:
    virtual void handleTraceEntry( const TraceEntry & );
    virtual void applyStorageConfiguration( const StorageConfiguration & );
    virtual void handleShutdownEvent( const ProcessShutdownEvent & );

    // Needed for the server to send out notifications to the GUI when entries are archived
    virtual void archivedEntries() {}
    // Needed for the server subclass to nuke the database
    void trimDb();
private:
    QSqlDatabase m_db;
    unsigned short m_shrinkBy;
    unsigned long m_maximumSize;
    QString m_archiveDir;
};

#endif // TRACER_DATABASEFEEDER_H
