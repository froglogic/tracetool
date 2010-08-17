#include "output.h"
#include "errorlog.h"

#include <assert.h>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

NetworkOutput::NetworkOutput( ErrorLog *errorLog, const string &remoteHost, unsigned short remotePort )
{
    // XXX implement
    assert( !"NetworkOutput class not implemented Unix" );
}

NetworkOutput::~NetworkOutput()
{
}

bool NetworkOutput::canWrite() const
{
}

void NetworkOutput::write( const vector<char> &data )
{
}

TRACELIB_NAMESPACE_END

