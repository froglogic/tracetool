/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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

