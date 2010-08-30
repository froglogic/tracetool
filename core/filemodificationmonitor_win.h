/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_FILEMODIFICATIONMONITOR_WIN_H
#define TRACELIB_FILEMODIFICATIONMONITOR_WIN_H

#include "tracelib_config.h"
#include "filemodificationmonitor.h"

#include <string>

#include <windows.h>

TRACELIB_NAMESPACE_BEGIN

class WinFileModificationMonitor : public FileModificationMonitor
{
public:
    WinFileModificationMonitor( const std::string &fileName,
                                FileModificationMonitorObserver *observer );
    virtual ~WinFileModificationMonitor();

    virtual bool start();

private:
    static DWORD WINAPI FilePollingThreadProc( LPVOID lpParameter );

    HANDLE m_pollingThread;
    HANDLE m_pollingThreadStopEvent;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_FILEMODIFICATIONMONITOR_WIN_H)

