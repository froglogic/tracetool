/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "filemodificationmonitor.h"

using namespace std;

TRACELIB_NAMESPACE_BEGIN

FileModificationMonitor::FileModificationMonitor( const string &fileName,
                                                  FileModificationMonitorObserver *observer )
    : m_fileName( fileName ),
    m_observer( observer )
{
}

void FileModificationMonitor::notifyObserver( FileModificationMonitorObserver::NotificationReason reason )
{
    if ( m_observer ) {
        m_observer->handleFileModification( m_fileName, reason );
    }
}

TRACELIB_NAMESPACE_END

