#include "output.h"

#include <map>

#include <assert.h>

using namespace std;
using namespace Tracelib;

class NetworkEventMonitor
{
public:
    static NetworkEventMonitor *self();

    ~NetworkEventMonitor();

    void addOutputObject( NetworkOutput *output, SOCKET socket );

private:
    NetworkEventMonitor();
    NetworkEventMonitor( const NetworkEventMonitor &other );
    void operator=( const NetworkEventMonitor &rhs );

    static LRESULT CALLBACK networkWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
    static const unsigned int WM_TRACELIB_CONNECTIONUPDATE;
    static std::map<SOCKET, NetworkOutput *> *s_outputObjects;

    HWND m_commWindow;
};

const unsigned int NetworkEventMonitor::WM_TRACELIB_CONNECTIONUPDATE = ::RegisterWindowMessage( TEXT( "Tracelib_ConnectionUpdate" ) );

map<SOCKET, NetworkOutput *> *NetworkEventMonitor::s_outputObjects = 0;

NetworkEventMonitor *NetworkEventMonitor::self()
{
    static map<SOCKET, NetworkOutput *> outputObjects;
    s_outputObjects = &outputObjects;

    static NetworkEventMonitor instance;
    return &instance;
}

NetworkEventMonitor::NetworkEventMonitor()
    : m_commWindow( NULL )
{
    static HINSTANCE programInstance = ::GetModuleHandle( NULL );
    if ( !programInstance ) {
        OutputDebugStringA( "GetModuleHandle failed" );
    }

    static WNDCLASSEX networkClassInfo = {
        sizeof( WNDCLASSEX ),
        0,
        &NetworkEventMonitor::networkWindowProc,
        0,
        0,
        programInstance,
        NULL,
        NULL,
        NULL,
        NULL,
        TEXT("Tracelib_Network_Window")
    };
    if ( !::RegisterClassEx( &networkClassInfo ) ) {
        OutputDebugStringA( "RegisterClassEx failed" );
    }

    m_commWindow = ::CreateWindow( TEXT("Tracelib_Network_Window"),
                                   NULL,
                                   0,
                                   0, 0, 0, 0,
                                   HWND_MESSAGE,
                                   NULL,
                                   programInstance,
                                   NULL );
    if ( !m_commWindow ) {
        OutputDebugStringA( "CreateWindow failed" );
    }
}

NetworkEventMonitor::~NetworkEventMonitor()
{
    ::DestroyWindow( m_commWindow );
}

void NetworkEventMonitor::addOutputObject( NetworkOutput *output, SOCKET socket )
{
    if ( ::WSAAsyncSelect( socket, m_commWindow, WM_TRACELIB_CONNECTIONUPDATE, FD_CONNECT | FD_CLOSE ) != 0 ) {
        OutputDebugStringA( "WSAAsyncSelect failed" );
    }

    (*s_outputObjects)[socket] = output;
}

LRESULT CALLBACK NetworkEventMonitor::networkWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    if ( msg == WM_TRACELIB_CONNECTIONUPDATE ) {
        map<SOCKET, NetworkOutput *>::iterator it = s_outputObjects->find( (SOCKET)wparam );
        NetworkOutput *outputObject = it->second;
        if ( WSAGETSELECTEVENT( lparam ) == FD_CONNECT ) {
            switch( WSAGETSELECTERROR( lparam ) ) {
                case 0:
                    OutputDebugStringA( "Tracelib: connected to viewer!" );
                    outputObject->setConnected();
                    break;
                case WSAEAFNOSUPPORT:
                    OutputDebugStringA( "1" );
                    break;
                case WSAECONNREFUSED:
                    OutputDebugStringA( "Tracelib: failed to connect to viewer, trying again..." );
                    outputObject->tryToConnect();
                    break;
                case WSAENETUNREACH:
                    OutputDebugStringA( "3" );
                    break;
                case WSAEFAULT:
                    OutputDebugStringA( "4" );
                    break;
                case WSAEINVAL:
                    OutputDebugStringA( "5" );
                    break;
                case WSAEISCONN:
                    OutputDebugStringA( "6" );
                    break;
                case WSAEMFILE:
                    OutputDebugStringA( "7" );
                    break;
                case WSAENOBUFS:
                    OutputDebugStringA( "8" );
                    break;
                case WSAENOTCONN:
                    OutputDebugStringA( "9" );
                    break;
                case WSAETIMEDOUT:
                    OutputDebugStringA( "10" );
                    break;
            }
        } else if ( WSAGETSELECTEVENT( lparam ) == FD_CLOSE ) {
            s_outputObjects->erase( (SOCKET)wparam );
            outputObject->closeSocket();
            outputObject->setupSocket();
            outputObject->tryToConnect();
        }
    }
    return ::DefWindowProc( hwnd, msg, wparam, lparam );
}

NetworkOutput::NetworkOutput( const string &remoteHost, unsigned short remotePort )
    : m_remoteHost( remoteHost ),
    m_remotePort( remotePort ),
    m_socket( 0 ),
    m_connected( false )
{
    WSADATA wsaData;
    int err = ::WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if ( err != 0 ) {
        OutputDebugStringA( "WSAStartup failed" );
    }

    if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) {
        OutputDebugStringA( "Faile to get proper WinSock version" );
    }

    setupSocket();
    tryToConnect();
}

void NetworkOutput::closeSocket()
{
    m_connected = false;
    closesocket( m_socket );
    m_socket = INVALID_SOCKET;
}

void NetworkOutput::setupSocket()
{
    m_connected = false;

    m_socket = ::WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 );
    if ( m_socket == INVALID_SOCKET ) {
        OutputDebugStringA( "WSASocket failed" );
    }

    NetworkEventMonitor::self()->addOutputObject( this, m_socket );
}

void NetworkOutput::tryToConnect()
{
    hostent *localHost = gethostbyname( m_remoteHost.c_str() );
    char *localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
    struct sockaddr_in remoteAddr;
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_addr.s_addr = inet_addr( localIP );
    remoteAddr.sin_port = htons( m_remotePort );

    if ( ::WSAConnect( m_socket,
                  (const struct sockaddr* )&remoteAddr,
                  sizeof(remoteAddr),
                  NULL,
                  NULL,
                  NULL,
                  NULL ) == 0 || WSAGetLastError() != WSAEWOULDBLOCK ) {
        OutputDebugStringA( "funny thing while connecting" );
    }
}

NetworkOutput::~NetworkOutput()
{
    ::WSACleanup();
}

void NetworkOutput::write( const vector<char> &data )
{
    if ( m_connected ) {
        send( m_socket, &data[0], data.size(), 0 );
    }
}

