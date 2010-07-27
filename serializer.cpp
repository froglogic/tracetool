#include "serializer.h"
#include "tracelib.h"

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
        char timestamp[64] = { '\0' };
        strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S: ", localtime(&entry.timeStamp));
        str << timestamp;
    }

    str << "Process " << entry.processId << " (Thread " << entry.threadId << "): ";

    switch ( entry.tracePoint->type ) {
        case TracePoint::ErrorPoint:
            str << "[ERROR]";
            break;
        case TracePoint::DebugPoint:
            str << "[DEBUG]";
            break;
        case TracePoint::LogPoint:
            str << "[LOG]";
            break;
        case TracePoint::WatchPoint:
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
            str << ( *it )->name() << "=" << ( *it )->toString() << " ";
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

    vector<char> buf( result.begin(), result.end() );
    buf.push_back( '\0' );
    return buf;
}

vector<char> CSVSerializer::serialize( const TraceEntry &entry )
{
    ostringstream str;
    str << entry.tracePoint->verbosity << "," << escape( entry.tracePoint->sourceFile ) << "," << entry.tracePoint->lineno << "," << escape( entry.tracePoint->functionName );

    if ( entry.variables ) {
        VariableSnapshot::const_iterator it, end = entry.variables->end();
        for ( it = entry.variables->begin(); it != end; ++it ) {
            str << "," << escape( ( *it )->name() ) << "," << escape( ( *it )->toString() );
        }
    }

    const string result = str.str();

    vector<char> buf( result.begin(), result.end() );
    buf.push_back( '\0' );
    return buf;
}

string CSVSerializer::escape( const string &s ) const
{
    string v;
    v.reserve( s.size() );
    for ( string::size_type i = 0; i < s.size(); ++i ) {
        switch ( s[i] ) {
            case ',':
                v += "\\,";
                break;
            case '\\':
                v += "\\\\";
                break;
            default:
                v += s[i];
                break;
        }
    }
    return v;
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
    str << "<traceentry pid=\"" << entry.processId << "\" tid=\"" << entry.threadId << "\" time=\"" << entry.timeStamp << "\">";

    std::string indent;
    if ( m_beautifiedOutput ) {
        indent = "\n  ";
    }

    str << indent << "<type>" << entry.tracePoint->type << "</type>";
    str << indent << "<verbosity>" << entry.tracePoint->verbosity << "</verbosity>";
    str << indent << "<location lineno=\"" << entry.tracePoint->lineno << "\"><![CDATA[" << entry.tracePoint->sourceFile << "]]></location>";
    str << indent << "<function><![CDATA[" << entry.tracePoint->functionName << "]]></function>'";
    if ( entry.variables ) {
        str << indent << "<variables>";
        if ( m_beautifiedOutput ) {
            indent = "\n    ";
        }
        VariableSnapshot::const_iterator it, end = entry.variables->end();
        for ( it = entry.variables->begin(); it != end; ++it ) {
            str << indent << "<variable name=\"" << ( *it )->name() << "\"><![CDATA[" << ( *it )->toString() << "]]></variable>";
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
    if ( m_beautifiedOutput ) {
        indent = "\n";
    }
    str << indent << "</traceentry>";
    if ( m_beautifiedOutput ) {
        str << "\n";
    }

    const string result = str.str();

    vector<char> buf( result.begin(), result.end() );
    buf.push_back( '\0' );
    return buf;
}

TRACELIB_NAMESPACE_END

