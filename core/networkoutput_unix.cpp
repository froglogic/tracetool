#include "output.h"
#include "errorlog.h"

#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

static int connectTo( const string host, unsigned short port, ErrorLog *log )
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
    if ( connect( sock, (const sockaddr *)&server, sizeof ( server ) ) )
        return sock;

    log->write( "connect: %s\n", strerror( errno ) );
    return -1;
}

static int writeTo( int fd, const char *data, const int length, ErrorLog *log )
{
    int written = 0;
    do {
        int nr = write( fd, data + written, length - written );
        if ( nr > 0 )
            written += nr;
        else if ( nr < 0 && errno != EINTR )
            break;
    } while ( length > written );

    if ( length != written )
        log->write( "write: %s\n", strerror( errno ) );
    return written;
}

NetworkOutput::NetworkOutput( ErrorLog *log, const string &host, unsigned short port )
    : m_host( host ), m_port( port ), m_socket( -1 ), m_error_log( log )
{}

NetworkOutput::~NetworkOutput()
{
}

bool NetworkOutput::open()
{
    if ( m_socket == -1 )
        m_socket = connectTo( m_host, m_port, m_error_log );
    return m_socket != -1;
}

bool NetworkOutput::canWrite() const
{
    return m_socket != -1;
}

void NetworkOutput::write( const vector<char> &data )
{
    if ( m_socket != -1 ) {
        if ( writeTo( m_socket, &data[0], data.size(), m_error_log ) < data.size() ) {
            ::close( m_socket );
            m_socket = 1;
        }
    }
}

TRACELIB_NAMESPACE_END

