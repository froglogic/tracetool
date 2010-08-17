#ifndef TRACELIB_OUTPUT_H
#define TRACELIB_OUTPUT_H

#include "tracelib_config.h"

#include <vector>

TRACELIB_NAMESPACE_BEGIN

class Output
{
public:
    virtual ~Output();

    virtual bool canWrite() const { return true; }
    virtual void write( const std::vector<char> &data ) = 0;

protected:
    Output();

private:
    Output( const Output &rhs );
    void operator=( const Output &other );
};

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
#else
#  include "networkoutput_unix.h"
#endif

#endif // !defined(TRACELIB_OUTPUT_H)

