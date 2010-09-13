/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "eventthread_unix.h"
#include "mutex.h"

#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>

TRACELIB_NAMESPACE_BEGIN

typedef std::map<int, FileEventObserver *> FileObserverList;

static const char EndMessageLoop = 'e';
static const char AddReadWatch = 'r';
static const char AddWriteWatch = 'w';
static const char RemoveReadWatch = 's';
static const char RemoveWriteWatch = 'x'; // flush and remove
static const char KillWriteWatch = 'y'; // remove, drop buffer
static const char NoError = '0';


class CommunicationPipeObserver : public FileEventObserver
{
public:
    CommunicationPipeObserver();
    ~CommunicationPipeObserver();

    void handleFileEvent( int fd, EventWatch watch );
    void error( int fd, int err, EventWatch watch );

    int countMonitors();
    int getFDSets( fd_set *rfds, fd_set *wfds );

    pthread_t event_list_thread;

    int command_pipe[2];
    int confirm_pipe[2];
    Mutex pipe_mutex;

    FileObserverList m_read_list;
    FileObserverList m_write_list;
};


static void closePipe( int *fd )
{
    close( fd[0] );
    close( fd[1] );
    fd[0] = fd[1] = -1;
}

static int getFDSet( FileObserverList &list, fd_set *fds )
{
    int max = -1;
    FD_ZERO( fds );
    const FileObserverList::iterator e = list.end();
    for ( FileObserverList::iterator it = list.begin(); it != e; ++it ) {
        FD_SET( it->first, fds );
        if ( it->first > max )
            max = it->first;

    }
    return max;
}

static int
handleFDSet( FileObserverList &list, fd_set *fds, FileEventObserver::EventWatch watch )
{
    int handled = 0;

    const FileObserverList::iterator e = list.end();
    for ( FileObserverList::iterator it = list.begin(); it != e; ++it ) {
        if ( FD_ISSET( it->first, fds ) ) {
            it->second->handleFileEvent( it->first, watch );
            ++handled;
        }
    }

    return handled;
}

static bool
handleError( FileObserverList &list, FileEventObserver::EventWatch watch )
{
    const FileObserverList::iterator e = list.end();
    for ( FileObserverList::iterator it = list.begin(); it != e; ++it ) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        fd_set fds;
        FD_ZERO( &fds );
        FD_SET( it->first, &fds );

        int retval;
        do {
            if ( FileEventObserver::FileRead == watch )
                retval = select( it->first + 1, &fds, NULL, 0L, &tv );
            else
                retval = select( it->first + 1, NULL, &fds, 0L, &tv );
        } while ( retval < 0 && errno == EINTR );

        if ( retval < 0 && errno != EINTR ) {
            it->second->error( it->first, errno, watch );
            list.erase( it );
            return true;
        }
    }
    return false;
}

static void *unixEventProc( void *user_data )
{
    CommunicationPipeObserver *data = (CommunicationPipeObserver*)user_data;

    write( data->confirm_pipe[1], &NoError, 1 );

    fd_set rfds;
    fd_set wfds;

    int nds = data->getFDSets( &rfds, &wfds );
    while ( data->event_list_thread && nds > 0 ) {

        int retval = select( nds, &rfds, &wfds, NULL, NULL );
        if ( retval == -1 ) {
            if ( errno != EINTR ) {
                if ( !handleError( data->m_read_list, FileEventObserver::FileRead ) &&
                        !handleError( data->m_write_list, FileEventObserver::FileWrite ) ) {
                    fprintf( stderr, "Unknown error in %s: %s\n",
                            __FUNCTION__,
                            strerror( errno ) );
                    break;
                }
            }
        } else if ( retval > 0 ) {
            if ( FD_ISSET( data->command_pipe[0], &rfds ) ) {
                data->handleFileEvent( data->command_pipe[0], FileEventObserver::FileRead );
            } else {
                handleFDSet( data->m_read_list, &rfds, FileEventObserver::FileRead );
                handleFDSet( data->m_write_list, &wfds, FileEventObserver::FileWrite );
            }
        }

        nds = data->getFDSets( &rfds, &wfds );
    }

    write( data->confirm_pipe[1], &NoError, 1 );

    return NULL;
}

CommunicationPipeObserver::CommunicationPipeObserver()
{
    if ( pipe( command_pipe ) != 0 ) {
        command_pipe[0] = command_pipe[1] = -1;
        fprintf( stderr, "%s %s", __FUNCTION__, strerror( errno ) );
        return;
    }
    if ( pipe( confirm_pipe ) != 0 ) {
        confirm_pipe[0] = confirm_pipe[1] = -1;
        fprintf( stderr, "%s %s", __FUNCTION__, strerror( errno ) );
        goto pipe1_out;
    }

    m_read_list[command_pipe[0]] = this;

    if ( pthread_create( &event_list_thread, NULL, unixEventProc, this ) ) {
        fprintf( stderr, "Couldn't create the event thread" );
        goto pipe2_out;
    }

    char response;
    if ( read( confirm_pipe[0], &response, 1 ) == 1 ) {
        return; // success
    }

pipe2_out:
    closePipe( confirm_pipe );
pipe1_out:
    closePipe( command_pipe );
}

CommunicationPipeObserver::~CommunicationPipeObserver()
{
    closePipe( command_pipe );
    closePipe( confirm_pipe );
}

int CommunicationPipeObserver::countMonitors()
{
    return m_read_list.size() + m_write_list.size() - 1;
}

int CommunicationPipeObserver::getFDSets( fd_set *rfds, fd_set *wfds )
{
    int rmax = getFDSet( m_read_list, rfds );
    int wmax = getFDSet( m_write_list, wfds );

    return 1 + ( rmax > wmax ? rmax : wmax );
}

static void readFdObserver( int in, int *fd, FileEventObserver **obs )
{
    read( in, fd, sizeof ( int ) );
    read( in, obs, sizeof ( FileEventObserver* ) );
}

void CommunicationPipeObserver::handleFileEvent( int in, EventWatch /*watch*/ )
{
    FileEventObserver *obs;
    int fd;
    int nr;

    char cmd;
    if ( read( in, &cmd, 1 ) == 1 ) {
        switch ( cmd ) {
        case AddReadWatch:
            readFdObserver( in, &fd, &obs );
            m_read_list[fd] = obs;
            break;
        case AddWriteWatch:
            readFdObserver( in, &fd, &obs );
            m_write_list[fd] = obs;
            break;
        case RemoveReadWatch:
            readFdObserver( in, &fd, &obs );
            m_read_list.erase( fd );
            nr = countMonitors();
            write( confirm_pipe[1], &nr, sizeof ( int ) );
            break;
        case RemoveWriteWatch:
            // TODO: select on fd until all its buffers written
        case KillWriteWatch:
            readFdObserver( in, &fd, &obs );
            m_read_list.erase( fd );
            nr = countMonitors();
            write( confirm_pipe[1], &nr, sizeof ( int ) );
            break;
        case EndMessageLoop:
            event_list_thread = 0;
        }
    }
}

void CommunicationPipeObserver::error( int /*fd*/, int err, EventWatch /*watch*/ )
{
    /* TODO: handle unlikely error with pipe communication */
    fprintf( stderr, "%s: %s\n", __FUNCTION__, strerror( err ) );
}

EventThreadUnix::EventThreadUnix() : d( new CommunicationPipeObserver )
{
}

EventThreadUnix::~EventThreadUnix()
{
    delete d;
}

void EventThreadUnix::addFileEventObserver( int fd, FileEventObserver *observer,
        FileEventObserver::EventWatch watch )
{
    if ( running() ) {
        MutexLocker locker( d->pipe_mutex );

        fcntl( fd, F_SETFL, fcntl( fd , F_GETFL ) | O_NONBLOCK );

        if ( FileEventObserver::FileRead == watch )
            write( d->command_pipe[1], &AddReadWatch, 1 );
        else if ( FileEventObserver::FileWrite == watch )
            write( d->command_pipe[1], &AddWriteWatch, 1 );
        write( d->command_pipe[1], &fd, sizeof ( fd ) );
        write( d->command_pipe[1], &observer, sizeof ( observer ) );
    }
}

void EventThreadUnix::removeFileEventObserver( int fd,
        FileEventObserver *observer,
        FileEventObserver::EventWatch watch,
        bool flush )
{
    bool quit = false;

    if ( running() ) {
        MutexLocker locker( d->pipe_mutex );

        if ( FileEventObserver::FileRead == watch ) {
            write( d->command_pipe[1], &RemoveReadWatch, 1 );
        } else if ( FileEventObserver::FileWrite == watch ) {
            if ( flush )
                write( d->command_pipe[1], &RemoveWriteWatch, 1 );
            else
                write( d->command_pipe[1], &KillWriteWatch, 1 );
        }
        write( d->command_pipe[1], &fd, sizeof ( fd ) );
        write( d->command_pipe[1], &observer, sizeof ( observer ) );

        int response;
        read( d->confirm_pipe[0], &response, sizeof ( int ) );

        if ( response == 0 ) {
            stop();
            quit = true;
        }
    }

    if ( quit ) {
        // outside the mutex lock
        delete this;
        m_self = NULL;
    }
}

bool EventThreadUnix::running()
{
    return m_self && m_self->d->command_pipe[1] > -1;
}

void EventThreadUnix::stop()
{
    // assert( running() && pipe_mutex locked );
    int nr = write( d->command_pipe[1], &EndMessageLoop, 1 );

    char response;
    read( d->confirm_pipe[0], &response, 1 );
}

EventThreadUnix *EventThreadUnix::m_self;

TRACELIB_NAMESPACE_END

