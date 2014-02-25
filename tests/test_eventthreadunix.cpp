/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "config.h"
#include "tracelib.h"
#include "tracelib_config.h"
#include "eventthread_unix.h"
#include "filemodificationmonitor.h"
#include "output.h"
#include "log.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include <string>

using namespace std;

int g_failureCount = 0;
int g_verificationCount = 0;

template <typename T>
static void verify( const char *what, T expected, T actual )
{
    if ( !( expected == actual ) ) {
        cout << "FAIL: " << what << "; expected '" << boolalpha << expected << "', got '" << boolalpha << actual << "'" << endl;
        ++g_failureCount;
    }
    ++g_verificationCount;
}

static void *fakeServerProc( void *user_data )
{
    std::string *output = (std::string *)user_data;

    struct sockaddr_in server;
    int opt = 1;
    int server_sock = socket( AF_INET, SOCK_STREAM, 0 );
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( TRACELIB_DEFAULT_PORT );
    setsockopt( server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof ( opt ) );
    if ( bind( server_sock, (const sockaddr*)&server, sizeof ( server ) ) ) {
        perror( "bind" );
        close( server_sock );
        return NULL;
    }
    if ( listen( server_sock, 5 ) ) {
        perror( "listen" );
        close( server_sock );
        return NULL;
    }
    struct sockaddr_in from;
    socklen_t fromlen = sizeof ( from );
    int fd = accept( server_sock, (sockaddr*)&from, &fromlen );
    close( server_sock );

    do {
        char buf[32];
        int nr = read( fd, buf, sizeof ( buf ) - 1 );
        if ( nr <= 0 && errno != EINTR )
            break;
        buf[nr] = 0;
        *output += buf;

    } while ( true );

    return NULL;
}

TRACELIB_NAMESPACE_BEGIN

class FileObserver : public FileModificationMonitorObserver
{
public:
    std::string file_name;
    int state;
    FileObserver() : state( 0 ) {}
    void handleFileModification( const std::string &fileName, NotificationReason reason )
    {
        switch( state ) {
        case 0:
            verify( "FileObserver::handleFileModification() creation",
                    FileModificationMonitorObserver::FileAppeared,
                    reason );
            break;
        case 1:
            verify( "FileObserver::handleFileModification() modification",
                    FileModificationMonitorObserver::FileModified,
                    reason );
            break;
        case 2:
            verify( "FileObserver::handleFileModification() unlink",
                    FileModificationMonitorObserver::FileDisappeared,
                    reason );
            break;
        default:
            fprintf( stderr, "handleFileModification %d\n", reason );
            verify( "FileObserver::handleFileModification()",
                    false,
                    true );
        }
        state++;
    }
};

static void testFileNotification()
{
#if HAVE_INOTIFY_H
    const int sleeping = 1;
#else
    const int sleeping = 6;
#endif

    FileObserver *observer = new FileObserver;
    FileModificationMonitor *monitor = FileModificationMonitor::create(
            "/tmp/test_event.xml", observer );
    monitor->start();
    FILE *file = fopen( "/tmp/test_event.xml", "w+" );
    sleep( sleeping );
    fprintf( file, "<config/>\n" );
    fclose( file );
    sleep( sleeping );
    unlink( "/tmp/test_event.xml" );
    sleep( sleeping );

    delete monitor;

    verify( "FileObserver::state reached 3",
            3,
            observer->state );
    delete observer;
}

struct TimerObserver : public EventObserver
{
    int count;

    TimerObserver() : count( 0 ) {}

    void handleEvent( EventContext *ctx, Event *event )
    {
        verify( "TimerObserver::handleEvent event type TimerEventType",
                Event::TimerEventType,
                event->eventType() );
        if ( ++count == 5 )
            TimerTask( this ).exec( ctx );
    }
};

static void testTimers()
{
    TimerObserver *obs = new TimerObserver();
    EventThreadUnix::self()->postTask( new TimerTask( 250, obs ) );
    sleep( 2 );
    verify( "TimerObserver.count < 6",
            true,
            obs->count < 6 );
    delete obs;
}

static void testCommunication()
{
    verify( "Initial EventThreadUnix::running()",
            false,
            EventThreadUnix::running() );
    (void)EventThreadUnix::self();
    verify( "Initial EventThreadUnix::running()",
            true,
            EventThreadUnix::running() );

    // as single fd in event thread
    testFileNotification();

    NullLog error_log;
    std::string quick_fox = "The quick brown fox jumps over the lazy dog";
    NetworkOutput *net = new NetworkOutput( &error_log, "127.0.0.1", TRACELIB_DEFAULT_PORT );
    verify( "Initial NetworkOutput::canWrite()",
            false,
            net->canWrite() );

    net->open();
    verify( "Connecting NetworkOutput::canWrite()",
            true,
            net->canWrite() );

    sleep( 2 );
    verify( "Connect Error NetworkOutput::canWrite()",
            false,
            net->canWrite() );

    delete net;

    std::string output, expected;
    pthread_t server_thread;
    pthread_create( &server_thread, NULL, fakeServerProc, &output );
    usleep( 100000 );

    net = new NetworkOutput( &error_log, "127.0.0.1", TRACELIB_DEFAULT_PORT );
    net->open();
    sleep( 1 );
    verify( "Connected NetworkOutput::canWrite()",
            true,
            net->canWrite() );

    for ( int i = 0; i < 5; ++i ) {
        expected += quick_fox;
        usleep( 1 );
        net->write( std::vector<char>( quick_fox.begin(), quick_fox.end() ) );
    }
    // as one of the fds in event thread
    testFileNotification();

    delete net;
    sleep( 1 );
    pthread_detach( server_thread );

    verify( "testCommunication server received",
            expected,
            output );

    verify( "EventThreadUnix::stop()",
            true,
            !EventThreadUnix::running() );

    // stress server, test terminating flushing buffers
    output = "";
    expected = "";
    quick_fox = quick_fox + quick_fox;
    quick_fox = quick_fox + quick_fox;
    quick_fox = quick_fox + quick_fox;
    quick_fox = quick_fox + quick_fox;
    pthread_create( &server_thread, NULL, fakeServerProc, &output );
    usleep( 100000 );

    net = new NetworkOutput( &error_log, "127.0.0.1", TRACELIB_DEFAULT_PORT );
    net->open();
    sleep( 1 );
    verify( "Connected NetworkOutput::canWrite()",
            true,
            net->canWrite() );

    for ( int i = 0; i < 5000; ++i ) {
        expected += quick_fox;
        net->write( std::vector<char>( quick_fox.begin(), quick_fox.end() ) );
    }

    delete net;
    sleep( 1 );
    pthread_detach( server_thread );

    verify( "testCommunication server received all data",
            true,
            output.size() == expected.size() );
    if ( output.size() == expected.size() )
        verify( "testCommunication server received correct data",
                true,
                expected == output );
    else
        verify( "testCommunication server received correct data start",
                true,
                output.size() > 0 &&
                expected.substr( 0, output.size() ) == output );

}

TRACELIB_NAMESPACE_END

int main()
{
    TRACELIB_NAMESPACE_IDENT(testCommunication)();
    TRACELIB_NAMESPACE_IDENT(testTimers)();
    cout << g_verificationCount << " verifications; " << g_failureCount << " failures found." << endl;
    return g_failureCount;
}

