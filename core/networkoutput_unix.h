#include "tracelib_config.h"

#include <assert.h>
#include <string>

TRACELIB_NAMESPACE_BEGIN

class ErrorLog;

class NetworkOutput : public Output
{
    std::string m_host;
    unsigned short m_port;
    int m_socket;
    ErrorLog *m_error_log;

public:
    NetworkOutput( ErrorLog *errorLog, const std::string &remoteHost, unsigned short remotePort );
    virtual ~NetworkOutput();

    virtual bool open();
    virtual bool canWrite() const;
    virtual void write( const std::vector<char> &data );
};

TRACELIB_NAMESPACE_END

