#include "serializer.h"
#include <ctime>
#include <sstream>

using namespace Tracelib;
using namespace std;

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
    char timestamp[64] = { '\0' };

    if ( m_showTimestamp ) {
        strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S: ", localtime(&entry.timeStamp));
    }

    string variableDump;
    if ( entry.variables && !entry.variables->empty() ) {
        variableDump = "; Variables: { ";
        vector<AbstractVariableConverter *>::const_iterator it, end = entry.variables->end();
        for ( it = entry.variables->begin(); it != end; ++it ) {
            variableDump += ( *it )->name();
            variableDump += "=";
            // XXX Consider encoding issues; (*it)->toString() yields UTF-8)
            variableDump += ( *it )->toString();
            variableDump += " ";
        }
        variableDump += "}";
    }

    string backtrace;
    if ( entry.backtrace ) {
        ostringstream str;
        str << "; Backtrace: { ";
        for ( size_t i = 0; i  < entry.backtrace->depth(); ++i ) {
            const StackFrame &frame = entry.backtrace->frame( i );
            // XXX Consider encoding issues; frame.module and frame.sourceFile
            // yield UTF-8
            str << "#" << i << ": in " << frame.module <<
                               ": " << frame.function << "+0x" << hex << frame.functionOffset
                               << " (" << frame.sourceFile << ":" << frame.lineNumber << ") ";
        }
        str << "}";
        backtrace = str.str();
    }

    vector<char> buf( 16384, '\0' ); // XXX don't hardcode buffer size

    // XXX Consider encoding issues
    _snprintf(&buf[0], buf.size(), "%s%s:%d: %s%s%s", timestamp, entry.tracePoint->sourceFile, entry.tracePoint->lineno, entry.tracePoint->functionName, variableDump.c_str(), backtrace.c_str() );
    return buf;
}

vector<char> CSVSerializer::serialize( const TraceEntry &entry )
{
    ostringstream str;
    // XXX Consider encoding issues
    str << entry.tracePoint->verbosity << "," << escape( entry.tracePoint->sourceFile ) << "," << entry.tracePoint->lineno << "," << escape( entry.tracePoint->functionName );

    if ( entry.variables ) {
        vector<AbstractVariableConverter *>::const_iterator it, end = entry.variables->end();
        for ( it = entry.variables->begin(); it != end; ++it ) {
            // XXX Consider encoding issues
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

