#include "tracelib_config.h"

/* Avoids that winsock.h is included by windows.h; winsock.h conflicts
 * with winsock2.h
 */
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <assert.h>

class NetworkEventMonitor;

TRACELIB_NAMESPACE_BEGIN

class ErrorLog;

class HandleOwner
{
public:
    HandleOwner( HANDLE handle = NULL ) : m_handle( handle ) { }
    ~HandleOwner() { if ( m_handle ) ::CloseHandle( m_handle ); }
    HandleOwner &operator=( HANDLE handle ) { assert( !m_handle ); m_handle = handle; return *this; }

    operator HANDLE() const { return m_handle; }

private:
    HandleOwner( const HandleOwner &other );
    void operator=( const HandleOwner &other );

    HANDLE m_handle;
};

class NetworkOutput : public Output
{
public:
    NetworkOutput( ErrorLog *errorLog, const std::string &remoteHost, unsigned short remotePort );
    virtual ~NetworkOutput();

    virtual bool canWrite() const;
    virtual void write( const std::vector<char> &data );

private:
    static DWORD __stdcall outputThreadProc( LPVOID param );

    HandleOwner m_outputObjectDestructingEvent;
    HandleOwner m_outputThread;
    HandleOwner m_outputThreadAttemptedConnectingEvent;
    std::string m_remoteHost;
    unsigned short m_remotePort;
    SOCKET m_socket;
    HandleOwner m_connectionEstablishedEvent;
    ErrorLog *m_errorLog;
};

TRACELIB_NAMESPACE_END

