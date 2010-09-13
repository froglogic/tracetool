/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H
#define TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H

#include "tracelib_config.h"
#include "filemodificationmonitor.h"
#include "eventthread_unix.h"

#include <string>

TRACELIB_NAMESPACE_BEGIN

class InotifyEventObserver;

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
    static InotifyEventObserver *inotify_instance;
    std::string base_name;
    int watch_descriptor;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H)

