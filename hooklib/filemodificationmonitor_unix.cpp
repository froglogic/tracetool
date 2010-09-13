/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "filemodificationmonitor_unix.h"

#include <cstdio>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/inotify.h>
#include <limits.h>
#include <libgen.h>
#include <map>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

class InotifyEventObserver : public FileEventObserver
{
public:
    InotifyEventObserver();
    ~InotifyEventObserver();

    int addWatch( const std::string &file_name, UnixFileModificationMonitor *obs );
    void removeWatch( int watch_descriptor );

    virtual void handleFileEvent( int fd, EventWatch watch );
    virtual void error( int fd, int err, EventWatch watch );

    typedef std::map<int, UnixFileModificationMonitor*> MonitorMap;
    MonitorMap monitor_map;
    int fd;
};

InotifyEventObserver::InotifyEventObserver() : fd( -1 )
{
    fd = inotify_init();
    if ( fd > -1 )
        EventThreadUnix::self()->addFileEventObserver( fd, this, FileRead );
}

InotifyEventObserver::~InotifyEventObserver()
{
    if ( fd > -1 ) {
        EventThreadUnix::self()->removeFileEventObserver( fd, this, FileRead );
        ::close( fd );
    }
}

int InotifyEventObserver::addWatch( const std::string &dir,
        UnixFileModificationMonitor *file_observer )
{
    int wd = -1;
    if ( fd > -1 ) {
        wd = inotify_add_watch( fd, dir.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY );
        if ( wd > -1 )
            monitor_map[wd] = file_observer;
    }
    return wd;
}

void InotifyEventObserver::removeWatch( int wd )
{
    if ( wd > -1 ) {
        inotify_rm_watch( fd, wd );
        monitor_map.erase( wd );
    }
}

void InotifyEventObserver::handleFileEvent( int fd, EventWatch watch )
{
    char buf[PATH_MAX + 1];
    if ( ::read( fd, buf, sizeof( buf ) ) > 0 ) {
        inotify_event *event = (inotify_event *)buf;
        MonitorMap::iterator it = monitor_map.find( event->wd );
        if ( it != monitor_map.end () && event->len > 0 ) {
            if ( event->mask & IN_CREATE )
                it->second->inotify( event->name, FileModificationMonitorObserver::FileAppeared );
            else if ( event->mask & ( IN_DELETE | IN_DELETE_SELF ) )
                it->second->inotify( event->name, FileModificationMonitorObserver::FileDisappeared );
            else if ( event->mask & IN_MODIFY )
                it->second->inotify( event->name, FileModificationMonitorObserver::FileModified );
            else
                fprintf( stderr, "unknown inotify event 0x%x\n", event->mask );
        }
    }
}

void InotifyEventObserver::error( int fd, int err, EventWatch watch )
{
}

InotifyEventObserver *UnixFileModificationMonitor::inotify_instance;


UnixFileModificationMonitor::UnixFileModificationMonitor( const string &fileName,
                                                          FileModificationMonitorObserver *observer )
    : FileModificationMonitor( fileName, observer ), watch_descriptor( -1 )
{
}

UnixFileModificationMonitor::~UnixFileModificationMonitor()
{
    if ( watch_descriptor > -1 )
        inotify_instance->removeWatch( watch_descriptor );

    if ( inotify_instance->monitor_map.size() == 0 ) {
        delete inotify_instance;
        inotify_instance = NULL;
    }
}

bool UnixFileModificationMonitor::start()
{
    if ( !inotify_instance )
        inotify_instance = new InotifyEventObserver;

    char *path = (char *)malloc( fileName().size() + 1 );

    memcpy( path, fileName().c_str(), fileName().size() + 1 );
    std::string dir_name = dirname( path );

    memcpy( path, fileName().c_str(), fileName().size() + 1 );
    base_name = basename( path );

    if ( dir_name.size() )
        watch_descriptor = inotify_instance->addWatch( dir_name, this );
    free( path );

    return watch_descriptor > -1;
}

void UnixFileModificationMonitor::inotify( const std::string &file,
        FileModificationMonitorObserver::NotificationReason reason )
{
    if ( base_name == file )
        notifyObserver( reason );
}

FileModificationMonitor *FileModificationMonitor::create( const string &fileName,
                                                          FileModificationMonitorObserver *observer )
{
    return new UnixFileModificationMonitor( fileName, observer );
}

TRACELIB_NAMESPACE_END

