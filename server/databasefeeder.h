/**********************************************************************
** Copyright ( C ) 2013 froglogic GmbH.
** All rights reserved.
**********************************************************************/
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
