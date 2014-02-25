/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifdef _WIN32
/* Avoids that winsock.h is included by windows.h; winsock.h conflicts
 * with winsock2.h
 */
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#endif

#include "output.h"
#include "log.h"

#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#ifdef _WIN32
#  include <windows.h>
#  include <winsock2.h>
#else
#  include <sys/socket.h>
#  include <unistd.h>
#  include <netdb.h>
#endif

#ifdef __FreeBSD__
#  include <netinet/in.h>
#endif

using namespace std;

TRACELIB_NAMESPACE_BEGIN

static int connectTo( const string host, unsigned short port, Log *log )
{
    struct hostent *he = gethostbyname( host.c_str() );
    if ( !he ) {
        log->write( "connect: host '%s' not found\n", host.c_str() );
        return -1;
    }

    struct sockaddr_in server;
    int sock = socket( AF_INET, SOCK_STREAM, 0 );
    server.sin_family = AF_INET;
    memcpy( &server.sin_addr.s_addr, he->h_addr, he->h_length );
    server.sin_port = htons( port );
    if ( !connect( sock, (const sockaddr *)&server, sizeof ( server ) ) )
        return sock;

    log->write( "connect: %s\n", strerror( errno ) );
    return -1;
}

static size_t writeTo( int fd, const char *data, const int length, Log *log )
{
    int written = 0;
    do {
#ifdef _WIN32
        int nr = send( fd, data + written, length - written, 0 );
#else
        int nr = send( fd, data + written, length - written, MSG_NOSIGNAL );
#endif
        if ( nr > 0 )
            written += nr;
        else if ( nr < 0 && errno != EINTR )
            break;
    } while ( length > written );

    if ( length != written )
        log->write( "write: %s\n", strerror( errno ) );
    assert( written >= 0 );
    return (size_t)written;
}

NetworkOutput::NetworkOutput( Log *log, const string &host, unsigned short port )
    : m_host( host ), m_port( port ), m_socket( -1 ), m_log( log ),
    m_lastConnectionAttemptFailed( false )
{
#ifdef _WIN32
    WSADATA wsaData;
    int err = ::WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
    if ( err != 0 ) {
        log->write( "WSAStartup failed!" );
        return;
    }

    if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) {
        log->write( "Failed to get proper WinSock version!" );
        return;
    }
#endif
}

NetworkOutput::~NetworkOutput()
{
    close();
#ifdef _WIN32
    ::WSACleanup();
#endif
}

bool NetworkOutput::open()
{
    if ( m_socket == -1 && !m_lastConnectionAttemptFailed ) {
        m_socket = connectTo( m_host, m_port, m_log );
        if ( m_socket == -1 ) {
            m_lastConnectionAttemptFailed = true;
        }
    }
    return m_socket != -1;
}

bool NetworkOutput::canWrite() const
{
    return m_socket != -1;
}

void NetworkOutput::write( const vector<char> &data )
{
    if ( m_socket != -1 ) {
        if ( writeTo( m_socket, &data[0], data.size(), m_log ) < data.size() ) {
            close();
        }
    }
}

void NetworkOutput::close()
{
#ifdef _WIN32
    closesocket( m_socket );
#else
    ::close( m_socket );
#endif
    m_socket = -1;
    m_lastConnectionAttemptFailed = false;
}

TRACELIB_NAMESPACE_END

