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

static const char PostTask = 'p';
static const char SendTask = 's';
static const char NoError = '0';


class EventContext : public FileEventObserver
{
public:
    EventContext();
    ~EventContext();

    void handleFileEvent( EventContext*, int fd, EventWatch watch );
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


class EndEventLoopTask : public Task
{
public:
    void *exec( EventContext *data )
    {
        data->event_list_thread = 0;
        return NULL;
    }
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
handleFDSet( EventContext *ctx, FileObserverList &list, fd_set *fds, FileEventObserver::EventWatch watch )
{
    int handled = 0;

    const FileObserverList::iterator e = list.end();
    for ( FileObserverList::iterator it = list.begin(); it != e; ) {
        FileObserverList::iterator next = it;
        ++next;
        if ( FD_ISSET( it->first, fds ) ) {
            it->second->handleFileEvent( ctx, it->first, watch );
            ++handled;
        }
        it = next;
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
    EventContext *data = (EventContext*)user_data;

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
                data->handleFileEvent( data, data->command_pipe[0], FileEventObserver::FileRead );
            } else {
                handleFDSet( data, data->m_read_list, &rfds, FileEventObserver::FileRead );
                handleFDSet( data, data->m_write_list, &wfds, FileEventObserver::FileWrite );
            }
        }

        nds = data->getFDSets( &rfds, &wfds );
    }

    write( data->confirm_pipe[1], &NoError, 1 );

    return NULL;
}

EventContext::EventContext()
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

EventContext::~EventContext()
{
    closePipe( command_pipe );
    closePipe( confirm_pipe );
}

int EventContext::countMonitors()
{
    return m_read_list.size() + m_write_list.size() - 1;
}

int EventContext::getFDSets( fd_set *rfds, fd_set *wfds )
{
    int rmax = getFDSet( m_read_list, rfds );
    int wmax = getFDSet( m_write_list, wfds );

    return 1 + ( rmax > wmax ? rmax : wmax );
}

void EventContext::handleFileEvent( EventContext*, int in, EventWatch/*watch*/ )
{
    char cmd;
    if ( read( in, &cmd, sizeof ( cmd ) ) == sizeof ( cmd ) ) {
        Task *task;
        switch ( cmd ) {
        case 'p':
            if ( read( in, &task, sizeof ( task ) ) == sizeof ( task ) ) {
                task->exec( this );
                delete task;
            }
            break;
        case 's':
            if ( read( in, &task, sizeof ( task ) ) == sizeof ( task ) ) {
                void *result = task->exec( this );
                write( confirm_pipe[1], &result, sizeof ( result ) );
            }
            break;
        }
    }
}

void EventContext::error( int /*fd*/, int err, EventWatch /*watch*/ )
{
    /* TODO: handle unlikely error with pipe communication */
    fprintf( stderr, "%s: %s\n", __FUNCTION__, strerror( err ) );
}

void *AddIOObserverTask::exec( EventContext *data )
{
    fcntl( fd, F_SETFL, fcntl( fd , F_GETFL ) | O_NONBLOCK );

    if ( watch_flags & FileEventObserver::FileRead ) {
        data->m_read_list[fd] = observer;
    }
    if ( watch_flags & FileEventObserver::FileWrite ) {
        data->m_write_list[fd] = observer;
    }
    return NULL;
}

void *RemoveIOObserverTask::exec( EventContext *data )
{
    if ( watch_flags & FileEventObserver::FileRead ) {
        data->m_read_list.erase( fd );
    }
    if ( watch_flags & FileEventObserver::FileWrite ) {
        data->m_write_list.erase( fd );
    }
    return (void *)(long) data->countMonitors();
}

void RemoveIOObserverTask::checkForLast()
{
    EventThreadUnix *thread = EventThreadUnix::self();
    if ( 0 == (long)thread->sendTask( this ) )
        delete thread;
}

EventThreadUnix::EventThreadUnix() : d( new EventContext )
{
}

EventThreadUnix::~EventThreadUnix()
{
    m_self = NULL;
    delete d;
}

void EventThreadUnix::postTask( Task *task )
{
    if ( running() ) {
        MutexLocker locker( d->pipe_mutex );
        write( d->command_pipe[1], &PostTask, sizeof ( PostTask ) );
        write( d->command_pipe[1], &task, sizeof ( task ) );
    }
}

void *EventThreadUnix::sendTask( Task *task )
{
    if ( running() ) {
        MutexLocker locker( d->pipe_mutex );

        write( d->command_pipe[1], &SendTask, sizeof ( SendTask ) );
        write( d->command_pipe[1], &task, sizeof ( task ) );

        void *response;
        read( d->confirm_pipe[0], &response, sizeof ( response ) );

        return response;
    }
    return NULL;
}

bool EventThreadUnix::running()
{
    return m_self && m_self->d->command_pipe[1] > -1;
}

void EventThreadUnix::stop()
{
    EndEventLoopTask task;
    sendTask( &task );
}

EventThreadUnix *EventThreadUnix::m_self;

TRACELIB_NAMESPACE_END

