/* Avoids that winsock.h is included by windows.h; winsock.h conflicts
 * with winsock2.h
 */
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>

class NetworkEventMonitor;

namespace Tracelib
{

class NetworkOutput : public Output
{
    friend class NetworkEventMonitor;
public:
    NetworkOutput( const std::string &remoteHost, unsigned short remotePort );
    virtual ~NetworkOutput();

    virtual bool canWrite() const { return m_connected; }
    virtual void write( const std::vector<char> &data );

private:
    void closeSocket();
    void setupSocket();
    void setConnected() { m_connected = true; }
    void tryToConnect();

    std::string m_remoteHost;
    unsigned short m_remotePort;
    SOCKET m_socket;
    bool m_connected;
};

}

