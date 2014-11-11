/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "serializer.h"
#include "trace.h"
#include "tracepoint.h"
#include "configuration.h"
#include "timehelper.h" // for timeToString

#include <algorithm> // for std::replace

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
        str << timeToString( entry.timeStamp ) << ": ";
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

    if ( entry.variables && entry.variables->size() > 0 ) {
        str << "; Variables: { ";
        for ( size_t i = 0; i < entry.variables->size(); ++i ) {
            AbstractVariable *v = (*entry.variables)[i];
            str << v->name() << "=" << convertVariableValue( v->value() ) << " ";
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

// From variabledumping.cpp, cannot easily share through the variabledumping header
// as that would make STL part of our API which is problematic
extern std::string stringRep( const VariableValue &v );

string PlaintextSerializer::convertVariableValue( const VariableValue &v ) const
{
    ostringstream str;
    str << stringRep ( v );
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

static std::string splitCDataEndToken( const std::string& input )
{
    std::string copy = input;
    static const char endToken[] = "]]>";
    static const char splitEndToken[] = "]]]]><![CDATA[>";
    static const size_t endTokenLength = strlen( endToken );
    static const size_t splitEndTokenLength = strlen( splitEndToken );
    size_t pos = 0;
    while ( ( pos = copy.find( endToken, pos ) ) != std::string::npos ) {
        copy.replace( pos, endTokenLength, splitEndToken );
        pos += splitEndTokenLength;
    }
    return copy;
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
    str << indent << "<processname><![CDATA[" << splitCDataEndToken( myProcessName ) << "]]></processname>";

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
            str << indent << "<key enabled=\"" << ( it->enabled ? "true" : "false" ) << "\"><![CDATA[" << splitCDataEndToken( it->name ) << "]]></key>";
        }
        if ( m_beautifiedOutput ) {
            indent = "\n  ";
        }
        str << indent << "</tracekeys>";
    }
    str << indent << "<type>" << entry.tracePoint->type << "</type>";
    str << indent << "<location lineno=\"" << entry.tracePoint->lineno << "\"><![CDATA[" << splitCDataEndToken( entry.tracePoint->sourceFile ) << "]]></location>";
    str << indent << "<function><![CDATA[" << splitCDataEndToken( entry.tracePoint->functionName ) << "]]></function>";
    if ( entry.variables ) {
        str << indent << "<variables>";
        if ( m_beautifiedOutput ) {
            indent = "\n    ";
        }
        for ( size_t i = 0; i < entry.variables->size(); ++i ) {
            AbstractVariable *v = (*entry.variables)[i];
            str << indent << convertVariable( v->name(), v->value() );
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
            str << indent << "<module><![CDATA[" << splitCDataEndToken( frame.module ) << "]]></module>";
            str << indent << "<function offset=\"" << frame.functionOffset << "\"><![CDATA[" << splitCDataEndToken( frame.function ) << "]]></function>";
            str << indent << "<location lineno=\"" << frame.lineNumber << "\"><![CDATA[" << splitCDataEndToken( frame.sourceFile ) << "]]></location>";

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
        str << indent << "<message><![CDATA[" << splitCDataEndToken( entry.message ) << "]]></message>";
    }

    str << indent << "<storageconfiguration"
                  << " maxSize=\"" << m_cfg.maximumTraceSize << "\""
                  << " shrinkBy=\"" << m_cfg.shrinkPercentage << "\""
                  << ">";
    if ( m_beautifiedOutput ) {
        indent += "  ";
    }
    str << indent << "<![CDATA[" << splitCDataEndToken( m_cfg.archiveDirectoryName ) << "]]>";
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
    str << "<![CDATA[" << splitCDataEndToken( myProcessName ) << "]]>";

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
            str << "type=\"string\"><![CDATA[" << splitCDataEndToken( v.asString() ) << "]]>";
            break;
        case VariableType::Number:
            str << "type=\"number\">";
            if( v.isSignedNumber() ) {
                str << static_cast<vlonglong>( v.asNumber() );
            } else {
                str << v.asNumber();
            }
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

