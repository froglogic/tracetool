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

vector<char> PlaintextSerializer::serialize( unsigned short verbosity,
                                             const char *sourceFile,
                                             unsigned int lineno,
                                             const char *functionName,
                                             const vector<AbstractVariableConverter *> &variables )
{
    char timestamp[64] = { '\0' };

    if ( m_showTimestamp ) {
        time_t t = time(NULL);
        strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S: ", localtime(&t));
    }

    string variableDump;
    if ( !variables.empty() ) {
        variableDump = "; Variables: { ";
        vector<AbstractVariableConverter *>::const_iterator it, end = variables.end();
        for ( it = variables.begin(); it != end; ++it ) {
            variableDump += ( *it )->name();
            variableDump += "=";
            variableDump += ( *it )->toString();
            variableDump += " ";
        }
        variableDump += "}";
    }

    vector<char> buf( 1024, '\0' ); // XXX don't hardcode buffer size
    _snprintf(&buf[0], buf.size(), "%s%s:%d: %s%s", timestamp, sourceFile, lineno, functionName, variableDump.c_str() );
    return buf;
}

vector<char> CSVSerializer::serialize( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName, const vector<AbstractVariableConverter *> &variables )
{
    ostringstream str;
    str << verbosity << "," << escape( sourceFile ) << "," << lineno << "," << escape( functionName );

    vector<AbstractVariableConverter *>::const_iterator it, end = variables.end();
    for ( it = variables.begin(); it != end; ++it ) {
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

