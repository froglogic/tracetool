/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_EVENTTHREAD_UNIX_H
#define TRACELIB_EVENTTHREAD_UNIX_H

#include "tracelib_config.h"

TRACELIB_NAMESPACE_BEGIN

class EventContext;

class FileEventObserver
{
public:
    enum EventWatch {
        NoWatch = 0,
        FileRead = 0x01,
        FileWrite = 0x02,
        FileReadWrite = 0x03
    };

    virtual ~FileEventObserver() {}

    virtual void handleFileEvent( EventContext*, int fd, EventWatch watch ) = 0;
    virtual void error( int fd, int err, EventWatch watch ) = 0;

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

private:
    EventThreadUnix();

    void stop();

    EventContext *d;

    static EventThreadUnix *m_self;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_EVENTTHREAD_UNIX_H)

