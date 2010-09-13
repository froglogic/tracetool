/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_EVENTTHREAD_UNIX_H
#define TRACELIB_EVENTTHREAD_UNIX_H

#include "tracelib_config.h"

TRACELIB_NAMESPACE_BEGIN

class CommunicationPipeObserver;

class FileEventObserver
{
public:
    enum EventWatch {
        FileRead,
        FileWrite
    };

    virtual ~FileEventObserver() {}

    virtual void handleFileEvent( int fd, EventWatch watch ) = 0;
    virtual void error( int fd, int err, EventWatch watch ) = 0;

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

    void addFileEventObserver( int fd,
                               FileEventObserver *observer,
                               FileEventObserver::EventWatch watch );
    void removeFileEventObserver( int fd,
                                  FileEventObserver *observer,
                                  FileEventObserver::EventWatch watch,
                                  bool flush_buffers = true );

private:
    EventThreadUnix();

    void stop();

    CommunicationPipeObserver *d;

    static EventThreadUnix *m_self;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_EVENTTHREAD_UNIX_H)

