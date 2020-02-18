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

class XmlParseException : public std::runtime_error
{
public:
    XmlParseException( const QString &what, const QString &msg, int code )
        : std::runtime_error( what.toUtf8().constData() ),
        m_msg( msg ),
        m_code( code )
    {
    }
    ~XmlParseException() throw() { }

    const QString &parserMessage() const { return m_msg; }
    int parserCode() const { return m_code; }

private:
    QString m_msg;
    int m_code;
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
