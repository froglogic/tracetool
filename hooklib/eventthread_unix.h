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

#ifndef TRACELIB_EVENTTHREAD_UNIX_H
#define TRACELIB_EVENTTHREAD_UNIX_H

#include "tracelib_config.h"
#include "getcurrentthreadid.h"

TRACELIB_NAMESPACE_BEGIN

class EventContext;

class Event
{
public:
    enum EventType {
        FileEventType,
        TimerEventType
    };
    EventType eventType() const { return type; }
protected:
    Event( EventType t ) : type( t ) {}

private:
    EventType type;
};

class FileEvent : public Event
{
public:
    enum EventWatch {
        Error = 0,
        FileRead = 0x01,
        FileWrite = 0x02,
        FileReadWrite = 0x03
    };

    int fd;
    int err;
    EventWatch watch;

    FileEvent( int f, int e, EventWatch w )
        : Event( Event::FileEventType ),
          fd( f ), err( e ), watch( w )
    {}
};

class TimerEvent : public Event
{
public:
    TimerEvent() : Event( TimerEventType ) {}
};

class EventObserver
{
public:
    virtual ~EventObserver() {}

    virtual void handleEvent( EventContext *context, Event *event ) = 0;
};


class FileEventObserver : public EventObserver
{
public:
    virtual void handleEvent( EventContext*, Event *event ) = 0;
};

class Task
{
public:
    virtual ~Task() {}

    virtual void *exec( EventContext* ) = 0;
};

class AddIOObserverTask : public Task
{
    int fd;
    int watch_flags;
    FileEventObserver *observer;
public:
    AddIOObserverTask( int f, FileEventObserver *obs, int flags )
        : fd( f ), watch_flags( flags ), observer( obs )
    {}

    virtual void *exec( EventContext* );
};

class RemoveIOObserverTask : public Task
{
    int fd;
    int watch_flags;
    FileEventObserver *observer;
public:
    RemoveIOObserverTask( int f, FileEventObserver *obs, int flags )
        : fd( f ), watch_flags( flags ), observer( obs )
    {}

    void checkForLast();

    virtual void *exec( EventContext* );
};

class TimerTask : public Task
{
    int timeout;
    EventObserver *observer;
    bool add;
public:
    TimerTask( int to, EventObserver *obs )
        : timeout( to ), observer( obs ), add( true )
    {}
    TimerTask( EventObserver *obs )
        : timeout( 0 ), observer( obs ), add( false )
    {}

    virtual void *exec( EventContext* );
};

class EventThreadUnix
{
public:
    ~EventThreadUnix();

    static EventThreadUnix *self()
    {
        if ( !m_self )
            m_self = new EventThreadUnix();
        if ( !m_self->running() ) {
            delete m_self;
            m_self = (EventThreadUnix *)0;
        }
        return m_self;
    }

    static bool running();

    void postTask( Task *task );
    void *sendTask( Task *task );
    void commandChannels( int *in, int *out );

    ThreadId threadId() const;
    EventContext *getContext() const { return d; }

    /* only run this in the event thread, eg. in a handleEvent call */
    static int processEvents( EventContext *ctx );

private:
    EventThreadUnix();

    void stop();

    EventContext *d;

    static EventThreadUnix *m_self;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_EVENTTHREAD_UNIX_H)

