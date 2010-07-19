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

class NetworkOutput : public Output
{
public:
    NetworkOutput( const char *remoteHost, unsigned short remotePort );
    virtual ~NetworkOutput();

    virtual bool canWrite() const { return m_connected; }
    virtual void write( const std::vector<char> &data );

private:
    static LRESULT CALLBACK networkWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

    void closeSocket();
    void setupSocket();
    void setConnected() { m_connected = true; }
    void tryToConnect();

    const char *m_remoteHost;
    unsigned short m_remotePort;
    HWND m_commWindow;
    SOCKET m_socket;
    bool m_connected;
};

}

