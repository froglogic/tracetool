#ifndef OUTPUT_H
#define OUTPUT_H

#include "tracelib.h"

/* Avoids that winsock.h is included by windows.h; winsock.h conflicts
 * with winsock2.h
 */
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>

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
    virtual ~MultiplexingOutput();

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
    SOCKET m_socket;
};

}

#endif // !defined(OUTPUT_H)

