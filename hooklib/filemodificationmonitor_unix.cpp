/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "filemodificationmonitor_unix.h"

#include <cstdio>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

UnixFileModificationMonitor::UnixFileModificationMonitor( const string &fileName,
                                                          FileModificationMonitorObserver *observer )
    : FileModificationMonitor( fileName, observer )
{
}

bool UnixFileModificationMonitor::start()
{
    fprintf( stderr, "UnixFileModificationMonitor not implemented!\n" );
    return false;
}

FileModificationMonitor *FileModificationMonitor::create( const string &fileName,
                                                          FileModificationMonitorObserver *observer )
{
    return new UnixFileModificationMonitor( fileName, observer );
}

TRACELIB_NAMESPACE_END

