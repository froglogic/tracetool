#ifndef OUTPUT_H
#define OUTPUT_H

#include "tracelib.h"

#include <windows.h>

namespace Tracelib
{

class StdoutOutput : public Output
{
public:
    virtual void write( const std::vector<char> &data );
};

class MultiplexingOutput : public Output
{
public:
    void addOutput( Output *output );

    virtual void write( const std::vector<char> &data );

private:
    std::vector<Output *> m_outputs;
};

class NetworkOutput : public Output
{
public:
    NetworkOutput();
    virtual ~NetworkOutput();

    virtual void write( const std::vector<char> &data );

private:
    static LRESULT CALLBACK networkWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

    HWND m_commWindow;
};

}

#endif // !defined(OUTPUT_H)

