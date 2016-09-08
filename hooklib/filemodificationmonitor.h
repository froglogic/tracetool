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

#ifndef TRACELIB_FILEMODIFICATIONMONITOR_H
#define TRACELIB_FILEMODIFICATIONMONITOR_H

#include "tracelib_config.h"

#include <string>

TRACELIB_NAMESPACE_BEGIN

class FileModificationMonitorObserver
{
public:
    enum NotificationReason {
        FileAppeared,
        FileDisappeared,
        FileModified
    };

    virtual ~FileModificationMonitorObserver() { }

    virtual void handleFileModification( const std::string &fileName, NotificationReason reason ) = 0;
};

class FileModificationMonitor
{
public:
    static FileModificationMonitor *create( const std::string &fileName,
                                            FileModificationMonitorObserver *observer );

    virtual ~FileModificationMonitor() { }

    virtual bool start() = 0;

    const std::string &fileName() const { return m_fileName; }

protected:
    FileModificationMonitor( const std::string &fileName,
                             FileModificationMonitorObserver *observer );
    void notifyObserver( FileModificationMonitorObserver::NotificationReason reason );

private:
    FileModificationMonitor( const FileModificationMonitor &other ); // disabled
    void operator=( const FileModificationMonitor &rhs ); // disabled

    std::string m_fileName;
    FileModificationMonitorObserver *m_observer;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_FILEMODIFICATIONMONITOR_H)

