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
#if HAVE_INOTIFY_H
# include <sys/inotify.h>
#else
# include <sys/types.h>
# include <sys/stat.h>
#endif
#include <limits.h>
#include <libgen.h>
#include <map>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

#if HAVE_INOTIFY_H
class INotifyEventObserver : public FileEventObserver
{
public:
    INotifyEventObserver();
    ~INotifyEventObserver();

    int addWatch( const std::string &file_name, UnixFileModificationMonitor *obs );
    void removeWatch( int watch_descriptor );

    virtual void handleEvent( EventContext*, Event *event );

    typedef std::map<int, UnixFileModificationMonitor*> MonitorMap;
    MonitorMap monitor_map;
    int fd;
};

INotifyEventObserver::INotifyEventObserver() : fd( -1 )
{
    fd = inotify_init();
    if ( fd > -1 )
        EventThreadUnix::self()->postTask( new AddIOObserverTask( fd, this, FileEvent::FileRead ) );
}

INotifyEventObserver::~INotifyEventObserver()
{
    if ( fd > -1 ) {
        RemoveIOObserverTask task( fd, this, FileEvent::FileRead );
        task.checkForLast();
        ::close( fd );
    }
}

int INotifyEventObserver::addWatch( const std::string &dir,
        UnixFileModificationMonitor *file_observer )
{
    int wd = -1;
    if ( fd > -1 ) {
        wd = inotify_add_watch( fd, dir.c_str(), IN_CREATE | IN_DELETE |
                                IN_MODIFY | IN_ATTRIB );
        if ( wd > -1 )
            monitor_map[wd] = file_observer;
        else
            fprintf( stderr, "inotify_add_watch() failed with error number "
                     "%d.\n", errno );
    }
    return wd;
}

void INotifyEventObserver::removeWatch( int wd )
{
    if ( wd > -1 ) {
        inotify_rm_watch( fd, wd );
        monitor_map.erase( wd );
    }
}

void INotifyEventObserver::handleEvent( EventContext*, Event *event )
{
    FileEvent *fe = (FileEvent *)event;
    if ( FileEvent::Error == fe->watch ) {
    } else {
        char buf[PATH_MAX + 1];
        if ( ::read( fe->fd, buf, sizeof( buf ) ) > 0 ) {
            inotify_event *inev = (inotify_event *)buf;
            MonitorMap::iterator it = monitor_map.find( inev->wd );
            if ( it != monitor_map.end () && inev->len > 0 ) {
                if ( inev->mask & IN_CREATE )
                    it->second->inotify( inev->name,
                            FileModificationMonitorObserver::FileAppeared );
                else if ( inev->mask & ( IN_DELETE | IN_DELETE_SELF ) )
                    it->second->inotify( inev->name,
                            FileModificationMonitorObserver::FileDisappeared );
                else if ( inev->mask & ( IN_MODIFY | IN_ATTRIB ) )
                    it->second->inotify( inev->name,
                            FileModificationMonitorObserver::FileModified );
                else
                    fprintf( stderr, "unknown inotify event 0x%x\n", inev->mask );
            }
        }
    }
}

INotifyEventObserver *UnixFileModificationMonitor::notify_instance;

#else

class TimerEventObserver : public FileEventObserver
{
    UnixFileModificationMonitor *modification_monitor;
    time_t modification_time;
public:
    TimerEventObserver( UnixFileModificationMonitor *);
    ~TimerEventObserver();

    virtual void handleEvent( EventContext*, Event *event );
};

TimerEventObserver::TimerEventObserver( UnixFileModificationMonitor *m )
    : modification_monitor( m ), modification_time( 0 )
{
    struct stat st;

    std::string fn = modification_monitor->fileName();
    int success = ::stat( fn.c_str(), &st );
    if( success == 0 ) {
        modification_time = st.st_mtime;
    }
    EventThreadUnix::self()->postTask( new TimerTask( 5000, this ) );
}

TimerEventObserver::~TimerEventObserver()
{
    EventThreadUnix::self()->postTask( new TimerTask( this ) );
}

void TimerEventObserver::handleEvent( EventContext*, Event* )
{
    struct stat st;

    std::string fn = modification_monitor->fileName();
    int success = ::stat( fn.c_str(), &st );
    if ( modification_time ) {
        if ( success == -1 ) {
            modification_time = 0;
            modification_monitor->inotify( fn, FileModificationMonitorObserver::FileDisappeared );
        } else if ( st.st_mtime > modification_time ) {
            modification_time = st.st_mtime;
            modification_monitor->inotify( fn, FileModificationMonitorObserver::FileModified );
        }
    } else if ( success == 0 ) {
        modification_time = st.st_mtime;
        modification_monitor->inotify( fn, FileModificationMonitorObserver::FileAppeared );
    }
}

#endif

UnixFileModificationMonitor::UnixFileModificationMonitor( const string &fileName,
                                                          FileModificationMonitorObserver *observer )
    : FileModificationMonitor( fileName, observer ),
#if HAVE_INOTIFY_H
      watch_descriptor( -1 )
#else
      notify_instance( NULL )
#endif
{
}

UnixFileModificationMonitor::~UnixFileModificationMonitor()
{
    if ( notify_instance ) {
#if HAVE_INOTIFY_H
        if ( watch_descriptor > -1 )
            notify_instance->removeWatch( watch_descriptor );

        if ( notify_instance->monitor_map.size() == 0 ) {
            delete notify_instance;
            notify_instance = NULL;
        }
#else
        delete notify_instance;
#endif
    }
}

bool UnixFileModificationMonitor::start()
{
#if HAVE_INOTIFY_H
    if ( !notify_instance )
        notify_instance = new INotifyEventObserver;

    char *path = (char *)malloc( fileName().size() + 1 );

    memcpy( path, fileName().c_str(), fileName().size() + 1 );
    std::string dir_name = dirname( path );

    memcpy( path, fileName().c_str(), fileName().size() + 1 );
    base_name = basename( path );

    if ( dir_name.size() )
        watch_descriptor = notify_instance->addWatch( dir_name, this );
    free( path );

    return watch_descriptor > -1;
#else
    notify_instance = new TimerEventObserver( this );
    return true;
#endif
}

void UnixFileModificationMonitor::inotify( const std::string &file,
        FileModificationMonitorObserver::NotificationReason reason )
{
#if HAVE_INOTIFY_H
    if ( base_name == file )
#endif
        notifyObserver( reason );
}

FileModificationMonitor *FileModificationMonitor::create( const string &fileName,
                                                          FileModificationMonitorObserver *observer )
{
    return new UnixFileModificationMonitor( fileName, observer );
}

TRACELIB_NAMESPACE_END

