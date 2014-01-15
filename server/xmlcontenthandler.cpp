/**********************************************************************
** Copyright (C) 2013 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "xmlcontenthandler.h"

XmlContentHandler::XmlContentHandler( XmlParseEventsHandler *handler )
    : m_handler( handler ),
    m_inFrameElement( false )
{
}

void XmlContentHandler::addData( const QByteArray &data )
{
    m_xmlReader.addData( data );
}

void XmlContentHandler::continueParsing()
{
    while ( !m_xmlReader.atEnd() ) {
        m_xmlReader.readNext();
        switch ( m_xmlReader.tokenType( )) {
            case QXmlStreamReader::StartElement:
                handleStartElement();
                break;
            case QXmlStreamReader::Characters:
                m_s = m_xmlReader.text().toString();
                break;
            case QXmlStreamReader::EndElement:
                handleEndElement();
                break;
            default:
                break;
        }
    }
}

void XmlContentHandler::handleStartElement()
{
    const QXmlStreamAttributes atts = m_xmlReader.attributes();
    if ( m_xmlReader.name() == QLatin1String( "traceentry" ) ) {
        m_currentEntry = TraceEntry();
        m_currentEntry.pid = atts.value( QLatin1String( "pid" ) ).toString().toUInt();
        qulonglong datetime = atts.value( QLatin1String( "process_starttime" ) ).toString().toULongLong();
        qint64 signedDt = datetime;
        QDateTime dt = QDateTime::fromMSecsSinceEpoch( signedDt );
        m_currentEntry.processStartTime = dt;
        m_currentEntry.tid = atts.value( QLatin1String( "tid" ) ).toString().toUInt();
        m_currentEntry.timestamp = QDateTime::fromMSecsSinceEpoch( atts.value( QLatin1String( "time" ) ).toString().toULongLong() );
    } else if ( m_xmlReader.name() == QLatin1String( "variable" ) ) {
        m_currentVariable = Variable();
        m_currentVariable.name = atts.value( QLatin1String( "name" ) ).toString();
        const QStringRef typeStr = atts.value( QLatin1String( "type" ) );
        if ( typeStr == QLatin1String( "string" ) ) {
            m_currentVariable.type = TRACELIB_NAMESPACE_IDENT(VariableType)::String;
        } else if ( typeStr == QLatin1String( "number" ) ) {
            m_currentVariable.type = TRACELIB_NAMESPACE_IDENT(VariableType)::Number;
        } else if ( typeStr == QLatin1String( "float" ) ) {
            m_currentVariable.type = TRACELIB_NAMESPACE_IDENT(VariableType)::Float;
        } else if ( typeStr == QLatin1String( "boolean" ) ) {
            m_currentVariable.type = TRACELIB_NAMESPACE_IDENT(VariableType)::Boolean;
        }
    } else if ( m_xmlReader.name() == QLatin1String( "location" ) ) {
        m_currentLineNo = atts.value( QLatin1String( "lineno" ) ).toString().toULong();
    } else if ( m_xmlReader.name() == QLatin1String( "frame" ) ) {
        m_inFrameElement = true;
        m_currentFrame = StackFrame();
    } else if ( m_xmlReader.name() == QLatin1String( "function" ) ) {
        m_currentFrame.functionOffset = atts.value( QLatin1String( "offset" ) ).toString().toUInt();
    } else if ( m_xmlReader.name() == QLatin1String( "shutdownevent" ) ) {
        m_currentShutdownEvent = ProcessShutdownEvent();
        m_currentShutdownEvent.pid = atts.value( QLatin1String( "pid" ) ).toString().toUInt();
        m_currentShutdownEvent.startTime = QDateTime::fromMSecsSinceEpoch( atts.value( QLatin1String( "starttime" ) ).toString().toULongLong() );
        m_currentShutdownEvent.stopTime = QDateTime::fromMSecsSinceEpoch( atts.value( QLatin1String( "endtime" ) ).toString().toULongLong() );
    } else if ( m_xmlReader.name() == QLatin1String( "storageconfiguration" ) ) {
        m_currentStorageConfig = StorageConfiguration();
        m_currentStorageConfig.maximumSize = atts.value( QLatin1String( "maxSize" ) ).toString().toULong();
        m_currentStorageConfig.shrinkBy = atts.value( QLatin1String( "shrinkBy" ) ).toString().toUInt();
    } else if ( m_xmlReader.name() == QLatin1String( "key" ) ) {
        m_currentTraceKey = TraceKey();
        m_currentTraceKey.enabled = atts.value( QLatin1String( "enabled" ) ) == QLatin1String( "true" );
    }
}

void XmlContentHandler::handleEndElement()
{
    if ( m_xmlReader.name() == QLatin1String( "traceentry" ) ) {
        m_handler->handleTraceEntry( m_currentEntry );
    } else if ( m_xmlReader.name() == QLatin1String( "variable" ) ) {
        m_currentVariable.value = m_s.trimmed();
        m_currentEntry.variables.append( m_currentVariable );
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "processname" ) ) {
        m_currentEntry.processName = m_s.trimmed();
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "stackposition" ) ) {
        m_currentEntry.stackPosition = m_s.trimmed().toULong();
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "type" ) ) {
        m_currentEntry.type = m_s.trimmed().toUInt();
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "location" ) ) {
        if ( m_inFrameElement ) {
            m_currentFrame.sourceFile = m_s.trimmed();
            m_currentFrame.lineNumber = m_currentLineNo;
        } else {
            m_currentEntry.path = m_s.trimmed();
            m_currentEntry.lineno = m_currentLineNo;
        }
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "group" ) ) {
        m_currentEntry.groupName = m_s.trimmed();
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "function" ) ) {
        m_currentEntry.function = m_s.trimmed();
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "message" ) ) {
        m_currentEntry.message = m_s.trimmed();
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "module" ) ) {
        m_currentFrame.module = m_s.trimmed();
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "function" ) ) {
        m_currentFrame.function = m_s.trimmed();
        m_s.clear();
    } else if ( m_xmlReader.name() == QLatin1String( "frame" ) ) {
        m_inFrameElement = false;
        m_currentEntry.backtrace.append( m_currentFrame );
    } else if ( m_xmlReader.name() == QLatin1String( "shutdownevent" ) ) {
        m_currentShutdownEvent.name = m_s.trimmed();
        m_s.clear();
        m_handler->handleShutdownEvent( m_currentShutdownEvent );
    } else if ( m_xmlReader.name() == QLatin1String( "key" ) ) {
        m_currentTraceKey.name = m_s.trimmed();
        m_s.clear();
        m_currentEntry.traceKeys.append( m_currentTraceKey );
    } else if ( m_xmlReader.name() == QLatin1String( "storageconfiguration" ) ) {
        m_currentStorageConfig.archiveDir = m_s.trimmed();
        m_s.clear();
        m_handler->applyStorageConfiguration( m_currentStorageConfig );
    }
}
