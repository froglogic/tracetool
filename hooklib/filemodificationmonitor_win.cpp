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

#include "filemodificationmonitor_win.h"

#include <cassert>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

WinFileModificationMonitor::WinFileModificationMonitor( const string &fileName,
                                                        FileModificationMonitorObserver *observer )
    : FileModificationMonitor( fileName, observer ),
    m_pollingThread( 0 ),
    m_pollingThreadStopEvent( 0 )
{
}

WinFileModificationMonitor::~WinFileModificationMonitor()
{
    if ( m_pollingThreadStopEvent ) {
        ::SetEvent( m_pollingThreadStopEvent );
        if ( ::WaitForSingleObject( m_pollingThread, 2000 ) == WAIT_TIMEOUT ) {
            ::TerminateThread( m_pollingThread, 0 );
        }
        ::CloseHandle( m_pollingThread );
        ::CloseHandle( m_pollingThreadStopEvent );
    }
}

bool WinFileModificationMonitor::start()
{
    assert( !m_pollingThread );

    m_pollingThreadStopEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( !m_pollingThreadStopEvent ) {
        return false;
    }

    m_pollingThread = ::CreateThread( NULL, 0, FilePollingThreadProc, this, 0, NULL );
    if ( !m_pollingThread ) {
        ::CloseHandle( m_pollingThreadStopEvent );
        m_pollingThreadStopEvent = 0;
        return false;
    }

    return true;
}

DWORD WINAPI WinFileModificationMonitor::FilePollingThreadProc( LPVOID lpParameter )
{
    WinFileModificationMonitor *monitorObject = static_cast<WinFileModificationMonitor *>( lpParameter );
    struct FileInfo {
        FileInfo( const std::string &fileName )
            : fileExists( false )
        {
            HANDLE fileHandle = ::CreateFileA( fileName.c_str(),
                                               GENERIC_READ,
                                               FILE_SHARE_READ,
                                               NULL,
                                               OPEN_EXISTING,
                                               FILE_ATTRIBUTE_NORMAL,
                                               NULL );
            if ( fileHandle == INVALID_HANDLE_VALUE ) {
                switch ( ::GetLastError() ) {
                    case ERROR_FILE_NOT_FOUND:
                        fileExists = false;
                        break;
                    default:
                        // XXX Handle errors
                        break;
                }
            } else {
                fileExists = true;
                ::GetFileTime( fileHandle, NULL, NULL, &modificationTime );
                ::CloseHandle( fileHandle );
            }
        }
        bool fileExists;
        FILETIME modificationTime;
    };
    FileInfo lastFileInfo( monitorObject->fileName() );
    while ( ::WaitForSingleObject( monitorObject->m_pollingThreadStopEvent, 0 ) == WAIT_TIMEOUT ) {
        ::Sleep( 250 );

        FileInfo newFileInfo( monitorObject->fileName() );

        if( newFileInfo.fileExists != lastFileInfo.fileExists ) {
            FileModificationMonitorObserver::NotificationReason reason = lastFileInfo.fileExists
                                            ? FileModificationMonitorObserver::FileDisappeared
                                            : FileModificationMonitorObserver::FileAppeared;
            monitorObject->notifyObserver( reason );
            lastFileInfo = newFileInfo;
            continue;
        }

        if ( ::CompareFileTime( &lastFileInfo.modificationTime, &newFileInfo.modificationTime ) != 0 ) {
            monitorObject->notifyObserver( FileModificationMonitorObserver::FileModified );
            lastFileInfo = newFileInfo;
        }
    }
    return 0;
}

FileModificationMonitor *FileModificationMonitor::create( const string &fileName,
                                                          FileModificationMonitorObserver *observer )
{
    return new WinFileModificationMonitor( fileName, observer );
}

TRACELIB_NAMESPACE_END

