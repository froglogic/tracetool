#include "output.h"

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

NetworkOutput::NetworkOutput()
    : m_commWindow( 0 ),
    m_socket( 0 )
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

    m_socket = ::WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0 );
    if ( m_socket == INVALID_SOCKET ) {
        OutputDebugStringA( "WSASocket failed" );
    }

    if ( ::WSAAsyncSelect( m_socket, m_commWindow, WM_TRACELIB_CONNECTIONUPDATE, FD_CONNECT | FD_CLOSE ) != 0 ) {
        OutputDebugStringA( "WSAAsyncSelect failed" );
    }

    tryToConnect( m_socket );
}

void NetworkOutput::tryToConnect( SOCKET sock )
{
    hostent *localHost = gethostbyname( "" );
    char *localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
    struct sockaddr_in viewerAddr;
    viewerAddr.sin_family = AF_INET;
    viewerAddr.sin_addr.s_addr = inet_addr( localIP );
    viewerAddr.sin_port = htons(44123);

    if ( ::WSAConnect( sock,
                  (const struct sockaddr* )&viewerAddr,
                  sizeof(viewerAddr),
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

bool NetworkOutput::g_haveViewer = false;

void NetworkOutput::write( const vector<char> &data )
{
    if ( g_haveViewer ) {
        send( m_socket, &data[0], data.size(), 0 );
    }
}

LRESULT CALLBACK NetworkOutput::networkWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    if ( msg == WM_TRACELIB_CONNECTIONUPDATE ) {
        if ( WSAGETSELECTEVENT( lparam ) == FD_CONNECT ) {
            switch( WSAGETSELECTERROR( lparam ) ) {
                case 0:
                    OutputDebugStringA( "Tracelib: connected to viewer!" );
                    g_haveViewer = true;
                    break;
                case WSAEAFNOSUPPORT:
                    OutputDebugStringA( "1" );
                    break;
                case WSAECONNREFUSED:
                    OutputDebugStringA( "Tracelib: failed to connect to viewer, trying again..." );
                    tryToConnect( (SOCKET)wparam );
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
            g_haveViewer = false;
            tryToConnect( (SOCKET)wparam );
        }
    }
    return ::DefWindowProc( hwnd, msg, wparam, lparam );
}

