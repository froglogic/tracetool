/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "serializer.h"
#include "trace.h"
#include "tracepoint.h"
#include "configuration.h"

#include <ctime>
#include <sstream>

#include <assert.h>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

Serializer::Serializer()
{
}

Serializer::~Serializer()
{
}

// XXX duplicated in errorlog.cpp
static string timeToString( time_t t )
{
    char timestamp[64] = { '\0' };
    strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S", localtime(&t));
    return string( (const char *)&timestamp );
}

PlaintextSerializer::PlaintextSerializer()
    : m_showTimestamp( true )
{
}

void PlaintextSerializer::setTimestampsShown( bool timestamps )
{
    m_showTimestamp = timestamps;
}

vector<char> PlaintextSerializer::serialize( const TraceEntry &entry )
{
    ostringstream str;

    if ( m_showTimestamp ) {
        str << timeToString( entry.timeStamp );
        str << ": ";
    }

    str << "Process " << entry.process.id << " [started at " << timeToString( entry.process.startTime ) << "] (Thread " << entry.threadId << "): ";

    switch ( entry.tracePoint->type ) {
        case TracePointType::Error:
            str << "[ERROR]";
            break;
        case TracePointType::Debug:
            str << "[DEBUG]";
            break;
        case TracePointType::Log:
            str << "[LOG]";
            break;
        case TracePointType::Watch:
            str << "[WATCH]";
            break;
        default:
            assert( !"Unreachable" );
    }

    if ( entry.message ) {
        str << " '" << entry.message << "'";
    }

    str << " " << entry.tracePoint->sourceFile << ":" << entry.tracePoint->lineno << ": " << entry.tracePoint->functionName;

    if ( entry.variables && !entry.variables->empty() ) {
        str << "; Variables: { ";
        VariableSnapshot::const_iterator it, end = entry.variables->end();
        for ( it = entry.variables->begin(); it != end; ++it ) {
            str << ( *it )->name() << "=" << convertVariableValue( ( *it )->value() ) << " ";
        }
        str << "}";
    }

    if ( entry.backtrace ) {
        str << "; Backtrace: { ";
        for ( size_t i = 0; i  < entry.backtrace->depth(); ++i ) {
            const StackFrame &frame = entry.backtrace->frame( i );
            str << "#" << i << ": in " << frame.module <<
                               ": " << frame.function << "+0x" << hex << frame.functionOffset
                               << " (" << frame.sourceFile << ":" << frame.lineNumber << ") ";
        }
        str << "}";
    }

    const string result = str.str();

    return vector<char>( result.begin(), result.end() );
}

vector<char> PlaintextSerializer::serialize( const ProcessShutdownEvent &ev )
{
    ostringstream str;
    str << timeToString( ev.shutdownTime ) << ": Process " << ev.process->id
        << " [started at " << timeToString( ev.process->startTime ) << "]"
        << " finished";

    const string result = str.str();

    return vector<char>( result.begin(), result.end() );
}

string PlaintextSerializer::convertVariableValue( const VariableValue &v ) const
{
    ostringstream str;
    switch ( v.type() ) {
        case VariableType::String:
            str << v.asString();
            break;
        case VariableType::Number:
            str << v.asNumber();
            break;
        case VariableType::Float:
            str << v.asFloat();
            break;
        case VariableType::Boolean:
            str << v.asBoolean();
            break;
        default:
            assert( !"Unreachable" );
    }
    str << " <" << VariableType::valueAsString( v.type() ) << ">";
    return str.str();
}

XMLSerializer::XMLSerializer()
    : m_beautifiedOutput( true )
{
}

void XMLSerializer::setBeautifiedOutput( bool beautifiedOutput )
{
    m_beautifiedOutput = beautifiedOutput;
}

vector<char> XMLSerializer::serialize( const TraceEntry &entry )
{
    ostringstream str;
    str << "<traceentry pid=\"" << entry.process.id << "\" process_starttime=\"" << entry.process.startTime << "\" tid=\"" << entry.threadId << "\" time=\"" << entry.timeStamp << "\">";

    std::string indent;
    if ( m_beautifiedOutput ) {
        indent = "\n  ";
    }

    static string myProcessName = Configuration::currentProcessName();
    str << indent << "<processname><![CDATA[" << myProcessName << "]]></processname>";

    str << indent << "<stackposition>" << entry.stackPosition << "</stackposition>";
    if ( entry.tracePoint->groupName ) {
        str << indent << "<group>" << entry.tracePoint->groupName << "</group>";
    }
    if ( !entry.process.availableTraceKeys.empty() ) {
        str << indent << "<tracekeys>";
        if ( m_beautifiedOutput ) {
            indent = "\n    ";
        }
        vector<TraceKey>::const_iterator it, end = entry.process.availableTraceKeys.end();
        for ( it = entry.process.availableTraceKeys.begin(); it != end; ++it ) {
            str << indent << "<key><![CDATA[" << it->name << "]]></key>";
        }
        if ( m_beautifiedOutput ) {
            indent = "\n  ";
        }
        str << indent << "</tracekeys>";
    }
    str << indent << "<type>" << entry.tracePoint->type << "</type>";
    str << indent << "<verbosity>" << entry.tracePoint->verbosity << "</verbosity>";
    str << indent << "<location lineno=\"" << entry.tracePoint->lineno << "\"><![CDATA[" << entry.tracePoint->sourceFile << "]]></location>";
    str << indent << "<function><![CDATA[" << entry.tracePoint->functionName << "]]></function>";
    if ( entry.variables ) {
        str << indent << "<variables>";
        if ( m_beautifiedOutput ) {
            indent = "\n    ";
        }
        VariableSnapshot::const_iterator it, end = entry.variables->end();
        for ( it = entry.variables->begin(); it != end; ++it ) {
            str << indent << convertVariable( ( *it )->name(), ( *it )->value() );
        }
        if ( m_beautifiedOutput ) {
            indent = "\n  ";
        }
        str << indent << "</variables>";
    }

    if ( entry.backtrace ) {
        str << indent << "<backtrace>";
        for ( size_t i = 0; i  < entry.backtrace->depth(); ++i ) {
            const StackFrame &frame = entry.backtrace->frame( i );

            if ( m_beautifiedOutput ) {
                indent = "\n    ";
            }
            str << indent << "<frame>";

            if ( m_beautifiedOutput ) {
                indent = "\n      ";
            }
            str << indent << "<module><![CDATA[" << frame.module << "]]></module>";
            str << indent << "<function offset=\"" << frame.functionOffset << "\"><![CDATA[" << frame.function << "]]></function>";
            str << indent << "<location lineno=\"" << frame.lineNumber << "\"><![CDATA[" << frame.sourceFile << "]]></location>";

            if ( m_beautifiedOutput ) {
                indent = "\n    ";
            }
            str << indent << "</frame>";
        }
        if ( m_beautifiedOutput ) {
            indent = "\n  ";
        }
        str << indent << "</backtrace>";
    }

    if ( entry.message ) {
        str << indent << "<message><![CDATA[" << entry.message << "]]></message>";
    }

    str << indent << "<storageconfiguration"
                  << " maxSize=\"" << m_cfg.maximumTraceSize << "\""
                  << " shrinkBy=\"" << m_cfg.shrinkPercentage << "\""
                  << ">";
    if ( m_beautifiedOutput ) {
        indent += "  ";
    }
    str << indent << "<![CDATA[" << m_cfg.archiveDirectoryName << "]]>";
    if ( m_beautifiedOutput ) {
        indent = "\n  ";
    }
    str << indent << "</storageconfiguration>";

    if ( m_beautifiedOutput ) {
        indent = "\n";
    }
    str << indent << "</traceentry>";
    if ( m_beautifiedOutput ) {
        str << "\n";
    }

    const string result = str.str();
    return vector<char>( result.begin(), result.end() );
}

vector<char> XMLSerializer::serialize( const ProcessShutdownEvent &ev )
{
    ostringstream str;
    str << "<shutdownevent pid=\"" << ev.process->id << "\" starttime=\"" << ev.process->startTime << "\" endtime=\"" << ev.shutdownTime << "\">";

    static string myProcessName = Configuration::currentProcessName();
    str << "<![CDATA[" << myProcessName << "]]>";

    str << "</shutdownevent>";

    const string result = str.str();
    return vector<char>( result.begin(), result.end() );
}

string XMLSerializer::convertVariable( const char *n, const VariableValue &v ) const
{
    ostringstream str;
    str << "<variable name=\"" << n << "\" ";
    switch ( v.type() ) {
        case VariableType::String:
            str << "type=\"string\"><![CDATA[" << v.asString() + "]]>";
            break;
        case VariableType::Number:
            str << "type=\"number\">" << v.asNumber();
            break;
        case VariableType::Float:
            str << "type=\"float\">" << v.asFloat();
            break;
        case VariableType::Boolean:
            str << "type=\"boolean\">" << v.asBoolean();
            break;
        default:
            assert( !"Unreachable" );
    }
    str << "</variable>";
    return str.str();
}

TRACELIB_NAMESPACE_END

