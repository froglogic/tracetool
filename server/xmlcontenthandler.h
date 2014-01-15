/**********************************************************************
** Copyright (C) 2013 froglogic GmbH.
** All rights reserved.
**********************************************************************/
#ifndef TRACER_XMLCONTENTHANDLER_H
#define TRACER_XMLCONTENTHANDLER_H

#include "database.h"
#include <QXmlStreamReader>

struct StorageConfiguration
{
    static const unsigned long UnlimitedTraceSize = 0;

    StorageConfiguration()
        : maximumSize( UnlimitedTraceSize ),
          shrinkBy( 10 )
    { }

    unsigned long maximumSize;
    unsigned short shrinkBy;
    QString archiveDir;
};

class XmlParseEventsHandler
{
    friend class XmlContentHandler;
protected:
    virtual void handleTraceEntry( const TraceEntry& ) = 0;
    virtual void applyStorageConfiguration( const StorageConfiguration & ) = 0;
    virtual void handleShutdownEvent( const ProcessShutdownEvent & ) = 0;
};

class XmlContentHandler
{
public:
    XmlContentHandler( XmlParseEventsHandler *handler );

    void addData( const QByteArray &data );

    void continueParsing();

private:
    void handleStartElement();
    void handleEndElement();

    QXmlStreamReader m_xmlReader;
    XmlParseEventsHandler *m_handler;
    TraceEntry m_currentEntry;
    Variable m_currentVariable;
    QString m_s;
    unsigned long m_currentLineNo;
    StackFrame m_currentFrame;
    bool m_inFrameElement;
    ProcessShutdownEvent m_currentShutdownEvent;
    StorageConfiguration m_currentStorageConfig;
    TraceKey m_currentTraceKey;
};

#endif // TRACER_XMLCONTENTHANDLER_H
