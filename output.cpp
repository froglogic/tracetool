#include "output.h"

#include <winsock2.h>

using namespace Tracelib;
using namespace std;

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

    if ( ::WSAAsyncSelect( m_socket, m_commWindow, 0, FD_CONNECT ) != 0 ) {
        OutputDebugStringA( "WSAAsyncSelect failed" );
    }
}

NetworkOutput::~NetworkOutput()
{
    ::DestroyWindow( m_commWindow );
    ::WSACleanup();
}

void NetworkOutput::write( const vector<char> &data )
{
    fprintf(stdout, "%s\n", &data[0]);
}

LRESULT CALLBACK NetworkOutput::networkWindowProc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    return ::DefWindowProc( hwnd, msg, wparam, lparam );
}

