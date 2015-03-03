/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "output.h"
#include "log.h"
#include "eventthread_unix.h"

#include <arpa/inet.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <list>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

class NetworkOutputPrivate : public FileEventObserver {
public:
    typedef std::list< std::vector<char> * > BufferList;

    // Only used in event thread
    BufferList buffers;
    string host;
    unsigned short port;
    bool notify_on_close;
    bool dummy;
    int m_socket;
    Log *log;
    ssize_t buf_pos;
    int watching;

    enum ObserverState {
        NotConnected,
        Connecting, Connected,
        Closing,
        Error
    };
    ObserverState state;

    // Only used in NetworkOutput calling thread
    enum NetworkOutputState {
        Idle,
        Opened,
        Closed,
        Failure
    };
    NetworkOutputState network_state;

    NetworkOutputPrivate( const string h, unsigned short p, Log *log );
    ~NetworkOutputPrivate();

    // Only used in NetworkOutput calling thread
    void connect();
    void close();

    // Only used in event thread
    void clear();
    void addObserver( EventContext *ctx, int watch );
    void removeObserver( EventContext *ctx, int watch );
    void endClosing( EventContext *ctx );
    bool write( EventContext *ctx, std::vector<char>* buffer );
    void handleEvent( EventContext*, Event *event );
};

class WriteDataTask : public Task
{
    NetworkOutputPrivate *observer;
    vector<char> *data;
public:
    WriteDataTask( NetworkOutputPrivate *obs, vector<char> *d )
        : observer( obs ), data( d )
    {}

    void *exec( EventContext* );
};

class SocketClosingTask : public Task
{
    NetworkOutputPrivate *observer;
public:
    SocketClosingTask( NetworkOutputPrivate *obs ) : observer( obs )
    {}

    void *exec( EventContext* );
};


NetworkOutputPrivate::NetworkOutputPrivate( const string h, unsigned short p, Log *_log )
 : host( h ),
   port( p ),
   notify_on_close( true ),
   m_socket( -1 ),
   log( _log ),
   buf_pos( 0),
   watching( FileEvent::Error ),
   state( NotConnected ),
   network_state( Idle )
{}

NetworkOutputPrivate::~NetworkOutputPrivate()
{
    close();
}

void NetworkOutputPrivate::connect()
{
    // NetworkOutput calling thread, no event thread calls at this point
    state = Error;
    network_state = Opened;

    struct hostent *he = gethostbyname( host.c_str() );
    if ( !he ) {
        log->writeError( "connect: host '%s' not found\n", host.c_str() );
        return;
    }

    struct sockaddr_in server;
    m_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
    server.sin_family = AF_INET;
    memcpy( &server.sin_addr.s_addr, he->h_addr, he->h_length );
    server.sin_port = htons( port );

    fcntl( m_socket, F_SETFL, fcntl( m_socket , F_GETFL ) | O_NONBLOCK );

    if ( ::connect( m_socket, (const sockaddr *)&server, sizeof ( server ) ) == -1 &&
            errno == EINPROGRESS ) {
        watching = FileEvent::FileReadWrite;
        EventThreadUnix::self()->postTask(
                new AddIOObserverTask( m_socket, this, watching ) );

        state = Connecting;
    } else {
        log->writeError( "connect to %s: %s", host.c_str(), strerror( errno ) );
        ::close( m_socket );
        m_socket = -1;
    }
}

void NetworkOutputPrivate::addObserver( EventContext *ctx, int watch )
{
    AddIOObserverTask( m_socket, this, watch ).exec( ctx );
    watching |= watch;
}

void NetworkOutputPrivate::removeObserver( EventContext *ctx, int watch )
{
    RemoveIOObserverTask( m_socket, this, watch ).exec( ctx );
    watching &= ~watch;
}

void NetworkOutputPrivate::handleEvent( EventContext *ctx, Event *event )
{
    if ( event->eventType() == Event::FileEventType ) {
        FileEvent *fe = (FileEvent *)event;
        if ( FileEvent::FileWrite == fe->watch ) {
            if ( Connecting == state ) {
                state = Connected;
                removeObserver( ctx, FileEvent::FileRead );
            }
            if ( buffers.size() ) {
                int total_written = 0;
                BufferList::iterator e = buffers.end();
                for ( BufferList::iterator it = buffers.begin(); it != e; ) {
                    vector<char> *buf = *it;

                    int nr = ::write( fe->fd, &(*buf)[0] + buf_pos, buf->size() - buf_pos );
                    if ( nr <= 0 )
                        break;

                    total_written += nr;
                    buf_pos += nr;
                    if ( buf_pos != (ssize_t)buf->size() )
                        break;

                    delete buf;
                    it = buffers.erase( it );
                    buf_pos = 0;
                }
                if ( !total_written ) {
                    clear(); // clears buffers, FileWrite observer below removed
                    state = Error;
                }
            }
            if ( buffers.size() == 0 ) {
                removeObserver( ctx, FileEvent::FileWrite );
                if ( Closing == state ) {
                    endClosing( ctx );
                }
            }
        } else if ( FileEvent::FileRead == fe->watch ) {
            log->writeError( "Connect error to %s %d %d",
                    host.c_str(), fe->fd, m_socket );
            removeObserver( ctx, FileEvent::FileReadWrite );
            clear();
            state = Error;
        } else if ( FileEvent::Error == fe->watch ) {
            log->writeError( "Network error to %s: %s %d",
                    host.c_str(), strerror( fe->err ), fe->fd );
            if ( Connecting == state )
                removeObserver( ctx, FileEvent::FileWrite );
            clear();
            state = Error;
            watching = FileEvent::Error;
        }
    } else { //TimerEventType
        if ( Closing == state ) {
            endClosing( ctx );
        }
    }
}

bool NetworkOutputPrivate::write( EventContext *ctx, std::vector<char>* buffer )
{
    if ( state > NotConnected && state < Closing ) {
        buffers.push_back( buffer );
        if ( !(watching & FileEvent::FileWrite ) ) {
            buf_pos = 0;
            addObserver( ctx, FileEvent::FileWrite );
        }
        return true;
    }
    delete buffer;
    return false;
}

void NetworkOutputPrivate::close()
{
    if ( EventThreadUnix::self()->threadId() == getCurrentThreadId() ) {
        bool old_notify_on_close = notify_on_close;
        notify_on_close = false;
        EventContext *ctx = EventThreadUnix::self()->getContext();
        SocketClosingTask( this ).exec( ctx );
        while (NetworkOutputPrivate::Connected == state )
            EventThreadUnix::processEvents( ctx );
        notify_on_close = old_notify_on_close;
    } else {
        EventThreadUnix::self()->postTask( new SocketClosingTask( this ) );

        int in, out;
        void *response;
        EventThreadUnix::self()->commandChannels( &in, &out );

        read( in, &response, sizeof ( response ) );

        clear();
    }
}

void NetworkOutputPrivate::endClosing( EventContext *ctx )
{
    if ( watching != Error ) {
        removeObserver( ctx, watching );
        clear();
    }
    state = NotConnected;

    if ( notify_on_close ) {
        int in, out;
        void *response = 0;
        EventThreadUnix::self()->commandChannels( &in, &out );
        ::write( out, &response, sizeof ( response ) );

        TimerTask( this ).exec( ctx );
    }
}

void NetworkOutputPrivate::clear()
{
    if ( m_socket > -1 ) {
        ::close( m_socket );
        m_socket = -1;
        state = NotConnected;
    }
    BufferList::iterator e = buffers.end();
    for ( BufferList::iterator it = buffers.begin(); it != e; ) {
        delete *it;
        it = buffers.erase( it );
    }
}


void *WriteDataTask::exec( EventContext *ctx )
{
    return observer->write( ctx, data )
        ? (void*)(long)NetworkOutputPrivate::Opened
        : (void*)(long)NetworkOutputPrivate::Failure;
}


void *SocketClosingTask::exec( EventContext *ctx )
{
    if ( observer->buffers.size() > 0 ) {
        // try for 10s to flush remaining buffers
        observer->state = NetworkOutputPrivate::Closing;
        TimerTask( 10000, observer ).exec( ctx );
    } else {
        observer->endClosing( ctx );
    }
    return NULL;
}


NetworkOutput::NetworkOutput( Log *log, const string &host, unsigned short port )
    : m_host( host ), m_port( port ), m_socket( -1 ), m_log( log ),
    d( new NetworkOutputPrivate( host, port, log ) )
{
}

NetworkOutput::~NetworkOutput()
{
    delete d;
}

bool NetworkOutput::open()
{
    if ( d->network_state == NetworkOutputPrivate::Idle )
        d->connect();

    return NetworkOutputPrivate::Opened == d->network_state;
}

bool NetworkOutput::canWrite() const
{
    return NetworkOutputPrivate::Opened == d->network_state;
    //return d->m_socket != -1;
}

void NetworkOutput::write( const vector<char> &data )
{
    if ( NetworkOutputPrivate::Opened == d->network_state ) {
        vector<char> *buf = new vector<char>( data );
        //buf->swap( data );
        WriteDataTask task( d, buf );
        d->network_state =
            (NetworkOutputPrivate::NetworkOutputState)(long)
            EventThreadUnix::self()->sendTask( &task );
    }
}

void NetworkOutput::close()
{
    if ( NetworkOutputPrivate::Opened == d->network_state ) {
        d->network_state = NetworkOutputPrivate::Closed;
        d->close();
    }
}

TRACELIB_NAMESPACE_END

