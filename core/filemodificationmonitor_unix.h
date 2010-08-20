#ifndef TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H
#define TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H

#include "tracelib_config.h"
#include "filemodificationmonitor.h"

#include <string>

TRACELIB_NAMESPACE_BEGIN

class UnixFileModificationMonitor : public FileModificationMonitor
{
public:
    UnixFileModificationMonitor( const std::string &fileName,
                                 FileModificationMonitorObserver *observer );

    virtual bool start();
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_FILEMODIFICATIONMONITOR_UNIX_H)

