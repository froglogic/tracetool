#include "tracelib_config.h"

#include <assert.h>
#include <string>

TRACELIB_NAMESPACE_BEGIN

class ErrorLog;

class NetworkOutput : public Output
{
public:
    NetworkOutput( ErrorLog *errorLog, const std::string &remoteHost, unsigned short remotePort );
    virtual ~NetworkOutput();

    virtual bool canWrite() const;
    virtual void write( const std::vector<char> &data );
};

TRACELIB_NAMESPACE_END

