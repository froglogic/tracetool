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

