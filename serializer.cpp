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
        time_t t = time(NULL);
        strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S: ", localtime(&t));
    }

    string variableDump;
    if ( !entry.variables.empty() ) {
        variableDump = "; Variables: { ";
        vector<AbstractVariableConverter *>::const_iterator it, end = entry.variables.end();
        for ( it = entry.variables.begin(); it != end; ++it ) {
            variableDump += ( *it )->name();
            variableDump += "=";
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
            str << "#" << i << ": in " << frame.module <<
                               ": " << frame.function << "+0x" << hex << frame.functionOffset
                               << " (" << frame.sourceFile << ":" << frame.lineNumber << ") ";
        }
        str << "}";
        backtrace = str.str();
    }

    vector<char> buf( 16384, '\0' ); // XXX don't hardcode buffer size
    _snprintf(&buf[0], buf.size(), "%s%s:%d: %s%s%s", timestamp, entry.sourceFile, entry.lineno, entry.functionName, variableDump.c_str(), backtrace.c_str() );
    return buf;
}

vector<char> CSVSerializer::serialize( const TraceEntry &entry )
{
    ostringstream str;
    str << entry.verbosity << "," << escape( entry.sourceFile ) << "," << entry.lineno << "," << escape( entry.functionName );

    vector<AbstractVariableConverter *>::const_iterator it, end = entry.variables.end();
    for ( it = entry.variables.begin(); it != end; ++it ) {
        str << "," << escape( ( *it )->name() ) << "," << escape( ( *it )->toString() );
    }

    const string result = str.str();
    return vector<char>( result.begin(), result.end() );
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

