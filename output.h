#ifndef OUTPUT_H
#define OUTPUT_H

#include "tracelib_config.h"
#include "tracelib.h"

TRACELIB_NAMESPACE_BEGIN

class StdoutOutput : public Output
{
public:
    virtual void write( const std::vector<char> &data );
};

class MultiplexingOutput : public Output
{
public:
    virtual ~MultiplexingOutput();

    void addOutput( Output *output );

    virtual void write( const std::vector<char> &data );

private:
    std::vector<Output *> m_outputs;
};

TRACELIB_NAMESPACE_END

#ifdef _WIN32
#  include "networkoutput_win.h"
#endif

#endif // !defined(OUTPUT_H)

