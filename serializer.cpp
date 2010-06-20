#include "serializer.h"
#include <ctime>

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
                                             const char *functionName )
{
    char timestamp[64] = { '\0' };

    if ( m_showTimestamp ) {
        time_t t = time(NULL);
        strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S: ", localtime(&t));
    }

    vector<char> buf( 1024, '\0' ); // XXX don't hardcode buffer size
    _snprintf(&buf[0], buf.size(), "%s%s:%d: %s", timestamp, sourceFile, lineno, functionName);
    return buf;
}

