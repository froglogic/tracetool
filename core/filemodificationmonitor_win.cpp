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
    WinFileModificationMonitor *monitorObject = (WinFileModificationMonitor *)lpParameter;

    bool fileExists = false;
    FILETIME lastModificationTime = { 0 };
    while ( ::WaitForSingleObject( monitorObject->m_pollingThreadStopEvent, 0 ) == WAIT_TIMEOUT ) {
        ::Sleep( 250 );

        HANDLE fileHandle = ::CreateFileA( monitorObject->fileName().c_str(),
                                           GENERIC_READ,
                                           FILE_SHARE_READ,
                                           NULL,
                                           OPEN_EXISTING,
                                           FILE_ATTRIBUTE_NORMAL,
                                           NULL );
        if ( fileHandle == INVALID_HANDLE_VALUE ) {
            switch ( ::GetLastError() ) {
                case ERROR_FILE_NOT_FOUND:
                    if ( fileExists ) {
                        monitorObject->notifyObserver( FileModificationMonitorObserver::FileDisappeared );
                        fileExists = false;
                    }
                    break;
                default:
                    // XXX Handle errors
                    break;
            }
            continue;
        }

        if ( !fileExists ) {
            monitorObject->notifyObserver( FileModificationMonitorObserver::FileAppeared );
            fileExists = true;
            ::CloseHandle( fileHandle );
            continue;
        }

        FILETIME currentModificationTime;
        if ( !::GetFileTime( fileHandle, NULL, NULL, &currentModificationTime ) ) {
            // XXX Handle errors
            ::CloseHandle( fileHandle );
            continue;
        }

        ::CloseHandle( fileHandle );

        if ( ::CompareFileTime( &lastModificationTime, &currentModificationTime ) != 0 ) {
            monitorObject->notifyObserver( FileModificationMonitorObserver::FileModified );
            lastModificationTime = currentModificationTime;
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

