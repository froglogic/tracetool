/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include <list>
#include <map>

static bool operator < ( const timeval &tv1, const timeval &tv2 )
{
    return tv1.tv_sec < tv2.tv_sec ||
        ( tv1.tv_sec == tv2.tv_sec && tv1.tv_usec < tv2.tv_usec );
}

TRACELIB_NAMESPACE_BEGIN

typedef std::map<int, FileEventObserver *> FileObserverList;
typedef struct _TimeOut { EventObserver *observer; int timeout; } TimeOut;
typedef std::list<TimeOut> TimeOutList;
typedef std::map<timeval, TimeOutList> TimeOutMap;

static const char PostTask = 'p';
static const char SendTask = 's';
static const char NoError = '0';


class EventContext : public FileEventObserver
{
public:
    EventContext();
    ~EventContext();

    void handleEvent( EventContext*, Event *event );

    int countMonitors();
    int getFDSets( fd_set *rfds, fd_set *wfds );

    pthread_t event_list_thread;
    bool keep_running;

    int command_pipe[2];
    int confirm_pipe[2];
    Mutex pipe_mutex;

    FileObserverList m_read_list;
    FileObserverList m_write_list;

    TimeOutMap m_timeout_map;
};


class EndEventLoopTask : public Task
{
public:
    void *exec( EventContext *data )
    {
        if ( data->event_list_thread ) {
            data->keep_running = false;
            return (void*)1L;
        }
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
handleFDSet( EventContext *ctx, FileObserverList &list, fd_set *fds, FileEvent::EventWatch watch )
{
    int handled = 0;

    const FileObserverList::iterator e = list.end();
    for ( FileObserverList::iterator it = list.begin(); it != e; ) {
        int fd = it->first;
        EventObserver *obs = it->second;
        ++it;
        if ( FD_ISSET( fd, fds ) ) {
            FileEvent event( fd, 0, watch );
            obs->handleEvent( ctx, &event );
            ++handled;
        }
    }

    return handled;
}

static bool
handleError( EventContext *ctx, FileObserverList &list, FileEvent::EventWatch watch )
{
    const FileObserverList::iterator e = list.end();
    for ( FileObserverList::iterator it = list.begin(); it != e; ++it ) {
        int fd = it->first;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        fd_set fds;
        FD_ZERO( &fds );
        FD_SET( fd, &fds );

        int retval;
        do {
            if ( FileEvent::FileRead == watch )
                retval = select( fd + 1, &fds, NULL, 0L, &tv );
            else
                retval = select( fd + 1, NULL, &fds, 0L, &tv );
        } while ( retval < 0 && errno == EINTR );

        if ( retval < 0 && errno != EINTR ) {
            EventObserver *observer = it->second;
            list.erase( it );
            FileEvent event( fd, errno, FileEvent::Error );
            observer->handleEvent( ctx, &event );
            return true;
        }
    }
    return false;
}

static void
addToTimeOut( TimeOutMap &map, const timeval &now, EventObserver *obs, int ms )
{
    timeval next;
    timeval add;
    if ( ms >= 1000 ) {
        add.tv_sec = ms / 1000;
        ms %= 1000;
    } else {
        add.tv_sec = 0;
    }
    add.tv_usec = ms * 1000;
    timeradd( &now, &add, &next );

    TimeOutMap::iterator it = map.find( next );
    TimeOut timeout = { obs, ms };
    if ( it == map.end() ) {
        TimeOutList tl;
        tl.push_back( timeout );
        map[next] = tl;
    } else {
        it->second.push_back( timeout );
    }
}

static void removeFromTimeOut( TimeOutMap &map, const EventObserver *observer )
{
    const TimeOutMap::iterator e = map.end();
    for ( TimeOutMap::iterator it = map.begin(); it != e; )
    {
        TimeOutMap::iterator next = it;
        ++next;

        const TimeOutList::iterator te = it->second.end();
        for ( TimeOutList::iterator ti = it->second.begin(); ti != te; )
        {
            if ( ti->observer == observer ) {
                ti = it->second.erase( ti );
                if ( it->second.size() == 0 )
                    map.erase( it );
                //return; not yet guaranteed that an observer is add only once
            } else {
                ++ti;
            }
        }

        it = next;
    }
}

static void handleTimeout( EventContext *ctx, const timeval &now )
{
    const TimeOutMap::iterator e = ctx->m_timeout_map.end();
    for ( TimeOutMap::iterator i = ctx->m_timeout_map.begin(); i != e; ) {
        TimeOutMap::iterator succ = i;
        ++succ;

        if ( now < i->first )
            break;

        const TimeOutList::iterator te = i->second.end();
        for ( TimeOutList::iterator ti = i->second.begin(); ti != te; )
        {
            TimeOut timeout = *ti;
            ti = i->second.erase( ti );

            if ( timeout.timeout > 0 ) {
                addToTimeOut( ctx->m_timeout_map, now,
                        timeout.observer, timeout.timeout );
            }

            TimerEvent event;
            timeout.observer->handleEvent( ctx, &event );
        }

        ctx->m_timeout_map.erase( i );
        i = succ;
    }
}

static int processFds( EventContext *data, int nds, fd_set *rfds, fd_set *wfds )
{
    timeval tv;
    timeval *cur = NULL;
    TimeOutMap::iterator it = data->m_timeout_map.begin();
    if ( it != data->m_timeout_map.end() ) {
        timeval now;
        gettimeofday( &now, NULL );
        handleTimeout( data, now );

        it = data->m_timeout_map.begin();
        if ( it != data->m_timeout_map.end() ) {
            timersub( &it->first, &now, &tv );
            cur = &tv;
        }
    }

    int retval = select( nds, rfds, wfds, NULL, cur );
    if ( retval == -1 ) {
        if ( errno != EINTR ) {
            if ( !handleError( data, data->m_read_list, FileEvent::FileRead ) &&
                    !handleError( data, data->m_write_list, FileEvent::FileWrite ) ) {
                fprintf( stderr, "Unknown error in %s: %s\n",
                        __FUNCTION__,
                        strerror( errno ) );
                return -1;
            }
        }
        retval = 0; // tell caller we didn't do anything
    } else if ( retval > 0 ) {
        if ( FD_ISSET( data->command_pipe[0], rfds ) ) {
            FileEvent event( data->command_pipe[0], 0, FileEvent::FileRead );
            data->handleEvent( data, &event );
        } else {
            handleFDSet( data, data->m_read_list, rfds, FileEvent::FileRead );
            handleFDSet( data, data->m_write_list, wfds, FileEvent::FileWrite );
        }
    }
    return retval;
}

static void *unixEventProc( void *user_data )
{
    EventContext *data = (EventContext*)user_data;

    write( data->confirm_pipe[1], &NoError, 1 );

    fd_set rfds;
    fd_set wfds;

    int nds = data->getFDSets( &rfds, &wfds );
    while ( data->keep_running && nds > 0 ) {
        if ( processFds( data, nds, &rfds, &wfds ) < 0 )
            break;

        nds = data->getFDSets( &rfds, &wfds );
    }

    write( data->confirm_pipe[1], &NoError, 1 );

    return NULL;
}

EventContext::EventContext() : keep_running( true )
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
        event_list_thread = 0;
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
    // ### not really counting all timers
    return m_read_list.size() + m_write_list.size() - 1 + m_timeout_map.size();
}

int EventContext::getFDSets( fd_set *rfds, fd_set *wfds )
{
    int rmax = getFDSet( m_read_list, rfds );
    int wmax = getFDSet( m_write_list, wfds );

    return 1 + ( rmax > wmax ? rmax : wmax );
}

void EventContext::handleEvent( EventContext*, Event *event )
{
    FileEvent *fe = (FileEvent *)event;
    if ( FileEvent::Error == fe->watch ) {
        /* TODO: handle unlikely error with pipe communication */
        fprintf( stderr, "%s: %s\n", __FUNCTION__, strerror( fe->err ) );
    } else {
        char cmd;
        if ( read( fe->fd, &cmd, sizeof ( cmd ) ) == sizeof ( cmd ) ) {
            Task *task;
            switch ( cmd ) {
            case 'p':
                if ( read( fe->fd, &task, sizeof( task ) ) == sizeof( task ) ) {
                    task->exec( this );
                    delete task;
                }
                break;
            case 's':
                if ( read( fe->fd, &task, sizeof( task ) ) == sizeof( task ) ) {
                    void *result = task->exec( this );
                    write( confirm_pipe[1], &result, sizeof ( result ) );
                }
                break;
            }
        }
    }
}

void *AddIOObserverTask::exec( EventContext *data )
{
    fcntl( fd, F_SETFL, fcntl( fd , F_GETFL ) | O_NONBLOCK );

    if ( watch_flags & FileEvent::FileRead ) {
        data->m_read_list[fd] = observer;
    }
    if ( watch_flags & FileEvent::FileWrite ) {
        data->m_write_list[fd] = observer;
    }
    return NULL;
}

void *RemoveIOObserverTask::exec( EventContext *data )
{
    if ( watch_flags & FileEvent::FileRead ) {
        data->m_read_list.erase( fd );
    }
    if ( watch_flags & FileEvent::FileWrite ) {
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

void *TimerTask::exec( EventContext *data )
{
    if ( add ) {
        timeval now;
        gettimeofday( &now, NULL );
        addToTimeOut( data->m_timeout_map, now, observer, timeout );
    } else {
        removeFromTimeOut( data->m_timeout_map, observer );
    }
    return (void *)(long)data->countMonitors();
}

EventThreadUnix::EventThreadUnix() : d( new EventContext )
{
}

EventThreadUnix::~EventThreadUnix()
{
    stop();
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

void EventThreadUnix::commandChannels( int *in, int *out )
{
    if ( running() ) {
        *in = d->confirm_pipe[0];
        *out = d->confirm_pipe[1];
    } else {
        *in = -1;
        *out = -1;
    }
}

bool EventThreadUnix::running()
{
    return m_self && m_self->d->command_pipe[1] > -1;
}

void EventThreadUnix::stop()
{
    EndEventLoopTask task;
    if ( sendTask( &task ) ) {
        pthread_detach( d->event_list_thread );
        d->event_list_thread = 0;
    }
}

ThreadId EventThreadUnix::threadId() const
{
    // pthread_t should be considered an opaque type, so this may
    // not compile at all if it is a plain struct. Unfortunately
    // supporting that would require extensive changes to the codebase
    return (ThreadId)d->event_list_thread;
}

int EventThreadUnix::processEvents( EventContext *ctx )
{
    fd_set rfds;
    fd_set wfds;
    int nds = ctx->getFDSets( &rfds, &wfds );

    if ( !ctx->keep_running || nds <= 0 )
        return -1;

    return processFds( ctx, nds, &rfds, &wfds );
}

EventThreadUnix *EventThreadUnix::m_self;

TRACELIB_NAMESPACE_END

