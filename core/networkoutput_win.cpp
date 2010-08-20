#include "output.h"
#include "errorlog.h"

#include <assert.h>

using namespace std;

static void yieldWin32Error( TRACELIB_NAMESPACE_IDENT(ErrorLog) *errorLog, const char *what, DWORD code )
{
    LPVOID lpMsgBuf;

    ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR) &lpMsgBuf,
        0, NULL );

    errorLog->write( "Tracelib Network Output: %s (code %d): %s", what, code, (LPSTR)lpMsgBuf );

    ::LocalFree( lpMsgBuf ); // because we used FORMAT_MESSAGE_ALLOCATE_BUFFER
}

class SocketCloser
{
public:
    SocketCloser( SOCKET *sock ) : m_sock( sock ) { }
    ~SocketCloser() { closesocket( *m_sock ); *m_sock = INVALID_SOCKET; }

private:
    SOCKET *m_sock;
};

TRACELIB_NAMESPACE_BEGIN

DWORD __stdcall NetworkOutput::outputThreadProc( LPVOID param )
{
    NetworkOutput *outputObject = (NetworkOutput *)param;

    struct sockaddr_in remoteAddr;
    {
        hostent *host = gethostbyname( outputObject->m_remoteHost.c_str() );
        if ( !host ) {
            yieldWin32Error( outputObject->m_errorLog, "gethostbyname failed", ::WSAGetLastError() );
            return 4;
        }

        char *ipStr = inet_ntoa (*(struct in_addr *)*host->h_addr_list);
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_addr.s_addr = inet_addr( ipStr );
        remoteAddr.sin_port = htons( outputObject->m_remotePort );
    }

    HandleOwner socketStateChangedEvent = ::WSACreateEvent();
    if ( !socketStateChangedEvent ) {
        yieldWin32Error( outputObject->m_errorLog, "WSACreateEvent failed", ::WSAGetLastError() );
        return 2;
    }

    HANDLE interestingEvents[2] = { socketStateChangedEvent, outputObject->m_outputObjectDestructingEvent };
    static const int numEvents = sizeof(interestingEvents) / sizeof(interestingEvents[0]);

    for ( ;; ) {
        outputObject->m_socket = ::WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 );
        if ( outputObject->m_socket == INVALID_SOCKET ) {
            yieldWin32Error( outputObject->m_errorLog, "WSASocket failed", ::WSAGetLastError() );
            return 1;
        }

        SocketCloser socketCloser( &outputObject->m_socket );

        if ( ::WSAEventSelect( outputObject->m_socket, socketStateChangedEvent, FD_CONNECT | FD_CLOSE ) != 0 ) {
            yieldWin32Error( outputObject->m_errorLog, "WSAEventSelect failed", ::WSAGetLastError() );
            return 3;
        }

        bool connected = false;
        while ( !connected ) {
            if ( ::WSAConnect( outputObject->m_socket,
                        (const struct sockaddr* )&remoteAddr,
                        sizeof(remoteAddr),
                        NULL,
                        NULL,
                        NULL,
                        NULL ) != 0 && WSAGetLastError() != WSAEWOULDBLOCK ) {
                yieldWin32Error( outputObject->m_errorLog, "WSAConnect failed", ::WSAGetLastError() );
                return 5;
            }

            const DWORD waitResult = ::WaitForMultipleObjects(
                    numEvents,
                    interestingEvents,
                    FALSE,
                    INFINITE );
            assert( waitResult != WAIT_TIMEOUT );
            if ( waitResult == WAIT_FAILED ) {
                yieldWin32Error( outputObject->m_errorLog, "WaitForMultipleObjects failed", ::WSAGetLastError() );
            } else if ( waitResult >= WAIT_ABANDONED && waitResult <= WAIT_ABANDONED ) {
                // XXX What does this mean?
            } else {
                switch ( waitResult - WAIT_OBJECT_0 ) {
                    case 0:
                        WSANETWORKEVENTS ev;
                        if ( ::WSAEnumNetworkEvents( outputObject->m_socket, socketStateChangedEvent, &ev ) != 0 ) {
                            yieldWin32Error( outputObject->m_errorLog, "WSAEnumNetworkEvents failed", ::WSAGetLastError() );
                            break;
                        }
                        assert( ev.lNetworkEvents == FD_CONNECT );
                        ::SetEvent( outputObject->m_outputThreadAttemptedConnectingEvent );
                        if ( ev.iErrorCode[FD_CONNECT_BIT] == 0 ) {
                            OutputDebugStringA( "Tracelib: connected to viewer!" );
                            ::SetEvent( outputObject->m_connectionEstablishedEvent );
                            connected = true;
                        } else {
                            OutputDebugStringA( "Tracelib: connection to view failed, trying again..." );
                        }
                        break;
                    case 1:
                        ::ResetEvent( outputObject->m_connectionEstablishedEvent );
                        return 0;
                }
            }
        }

        const DWORD waitResult = ::WaitForMultipleObjects(
                numEvents,
                interestingEvents,
                FALSE,
                INFINITE );
        assert( waitResult != WAIT_TIMEOUT );
        if ( waitResult == WAIT_FAILED ) {
            yieldWin32Error( outputObject->m_errorLog, "WaitForMultipleObjects failed", ::WSAGetLastError() );
        } else if ( waitResult >= WAIT_ABANDONED && waitResult <= WAIT_ABANDONED ) {
            // XXX What does this mean?
        } else {
            switch ( waitResult - WAIT_OBJECT_0 ) {
                case 0:
                    WSANETWORKEVENTS ev;
                    if ( ::WSAEnumNetworkEvents( outputObject->m_socket, socketStateChangedEvent, &ev ) != 0 ) {
                        yieldWin32Error( outputObject->m_errorLog, "WSAEnumNetworkEvents failed", ::WSAGetLastError() );
                        break;
                    }
                    assert( ev.lNetworkEvents == FD_CLOSE );
                    OutputDebugStringA( "Tracelib: viewer went away!" );
                    ::ResetEvent( outputObject->m_connectionEstablishedEvent );
                    break;
                case 1:
                    ::ResetEvent( outputObject->m_connectionEstablishedEvent );
                    return 0;
            }
        }
    }
    assert( !"Unreachable" );
    return 0;
}

NetworkOutput::NetworkOutput( ErrorLog *errorLog, const string &remoteHost, unsigned short remotePort )
    : m_outputObjectDestructingEvent( 0 ),
    m_outputThread( 0 ),
    m_outputThreadAttemptedConnectingEvent( 0 ),
    m_remoteHost( remoteHost ),
    m_remotePort( remotePort ),
    m_socket( 0 ),
    m_connectionEstablishedEvent( 0 ),
    m_errorLog( errorLog )
{
    WSADATA wsaData;
    int err = ::WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if ( err != 0 ) {
        yieldWin32Error( m_errorLog, "WSAStartup failed", ::WSAGetLastError() );
        return;
    }

    if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) {
        OutputDebugStringA( "Failed to get proper WinSock version" );
        return;
    }

    m_outputThreadAttemptedConnectingEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( !m_outputThreadAttemptedConnectingEvent ) {
        yieldWin32Error( m_errorLog, "CreateEvent failed", ::GetLastError() );
        return;
    }

    m_outputObjectDestructingEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( !m_outputObjectDestructingEvent ) {
        yieldWin32Error( m_errorLog, "CreateEvent failed", ::GetLastError() );
        return;
    }

    m_connectionEstablishedEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( !m_connectionEstablishedEvent ) {
        yieldWin32Error( m_errorLog, "CreateEvent failed", ::GetLastError() );
        return;
    }

    m_outputThread = ::CreateThread( NULL, 0, &outputThreadProc, this, 0, NULL );
    if ( !m_outputThread ) {
        yieldWin32Error( m_errorLog, "CreateThread failed", ::GetLastError() );
        return;
    }

    HANDLE events[2] = { m_outputThread, m_outputThreadAttemptedConnectingEvent };
    // XXX Error handling!
    ::WaitForMultipleObjects( sizeof(events) / sizeof(events[0]),
                events,
                FALSE,
                INFINITE );
}

NetworkOutput::~NetworkOutput()
{
    ::SetEvent( m_outputObjectDestructingEvent );
    if ( ::WaitForSingleObject( m_outputThread, 2000 ) == WAIT_TIMEOUT ) {
        ::TerminateThread( m_outputThread, 127 );
    }
    ::WSACleanup();
}

bool NetworkOutput::canWrite() const
{
    return ::WaitForSingleObject( m_connectionEstablishedEvent, 0 ) == WAIT_OBJECT_0;
}

void NetworkOutput::write( const vector<char> &data )
{
    if ( canWrite() ) {
        send( m_socket, &data[0], data.size(), 0 );
    }
}

TRACELIB_NAMESPACE_END

