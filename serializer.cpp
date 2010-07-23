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

TRACELIB_NAMESPACE_END

