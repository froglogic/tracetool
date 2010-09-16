/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H
#define TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H

#include "config.h"
#include "tracelib_config.h"
#include "filemodificationmonitor.h"
#include "eventthread_unix.h"

#include <string>

TRACELIB_NAMESPACE_BEGIN

#ifdef HAVE_INOTIFY_H
class INotifyEventObserver;
#else
class TimerEventObserver;
#endif

class UnixFileModificationMonitor : public FileModificationMonitor
{
public:
    UnixFileModificationMonitor( const std::string &fileName,
                                 FileModificationMonitorObserver *observer );
    ~UnixFileModificationMonitor();

    virtual bool start();

    void inotify( const std::string &fileName,
                  FileModificationMonitorObserver::NotificationReason reason );

private:
#ifdef HAVE_INOTIFY_H
    static INotifyEventObserver *notify_instance;
    std::string base_name;
    int watch_descriptor;
#else
    TimerEventObserver *notify_instance;
#endif
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H)

