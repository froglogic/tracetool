#include "output.h"

#include <cassert>
#include <map>

using namespace Tracelib;
using namespace std;

static const unsigned int WM_TRACELIB_CONNECTIONUPDATE = ::RegisterWindowMessage( TEXT( "Tracelib_ConnectionUpdate" ) );

void StdoutOutput::write( const vector<char> &data )
{
    fprintf(stdout, "%s\n", &data[0]);
}

void MultiplexingOutput::addOutput( Output *output )
{
    m_outputs.push_back( output );
}

void MultiplexingOutput::write( const vector<char> &data )
{
    vector<Output *>::const_iterator it, end = m_outputs.end();
    for ( it = m_outputs.begin(); it != end; ++it ) {
        ( *it )->write( data );
    }
}

MultiplexingOutput::~MultiplexingOutput()
{
    vector<Output *>::const_iterator it, end = m_outputs.end();
    for ( it = m_outputs.begin(); it != end; ++it ) {
        delete *it;
    }
}

static map<SOCKET, NetworkOutput *> *g_networkOutputs = 0;

NetworkOutput::NetworkOutput( const char *remoteHost, unsigned short remotePort )
    : m_remoteHost( remoteHost ),
    m_remotePort( remotePort ),
    m_commWindow( 0 ),
    m_socket( 0 ),
    m_connected( false )
{
    static HINSTANCE programInstance = ::GetModuleHandle( NULL );
    if ( !programInstance ) {
        OutputDebugStringA( "GetModuleHandle failed" );
    }

    static WNDCLASSEX networkClassInfo = {
        sizeof( WNDCLASSEX ),
        0,
        &networkWindowProc,
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
    assert( g_networkOutputs );
    g_networkOutputs->erase( m_socket );
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

    if ( ::WSAAsyncSelect( m_socket, m_commWindow, WM_TRACELIB_CONNECTIONUPDATE, FD_CONNECT | FD_CLOSE ) != 0 ) {
        OutputDebugStringA( "WSAAsyncSelect failed" );
    }

    if ( !g_networkOutputs ) {
        g_networkOutputs = new map<SOCKET, NetworkOutput *>;
    }
    (*g_networkOutputs)[m_socket] = this;
}

void NetworkOutput::tryToConnect()
{
    hostent *localHost = gethostbyname( m_remoteHost );
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
    ::DestroyWindow( m_commWindow );
    ::WSACleanup();
}

void NetworkOutput::write( const vector<char> &data )
{
    if ( m_connected ) {
        send( m_socket, &data[0], data.size(), 0 );
    }
}

LRESULT CALLBACK NetworkOutput::networkWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    if ( msg == WM_TRACELIB_CONNECTIONUPDATE ) {
        assert( g_networkOutputs );
        map<SOCKET, NetworkOutput *>::iterator it = g_networkOutputs->find( (SOCKET)wparam );
        assert( it != g_networkOutputs->end() );
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
            OutputDebugStringA( "Viewer disappeared." );
            outputObject->closeSocket();
            outputObject->setupSocket();
            outputObject->tryToConnect();
        }
    }
    return ::DefWindowProc( hwnd, msg, wparam, lparam );
}

