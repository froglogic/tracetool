#ifndef ERRORLOG_H
#define ERRORLOG_H

#include <string>
#include <vector>

#ifdef _WIN32
#  include <windows.h>
#  define vsnprintf _vsnprintf
#endif

namespace Tracelib
{

class ErrorLog
{
public:
    virtual ~ErrorLog();

    virtual void write( const std::string &msg ) = 0;
    inline void write( const char *format, ... ) {
        va_list argList;
        va_start( argList, format );

        std::vector<char> buf( 1024, '\0' );
        vsnprintf( &buf[0], buf.size() - 1, format, argList );
        va_end( argList );
        write( std::string( &buf[0] ) );
    }

protected:
    ErrorLog();

private:
    ErrorLog( const ErrorLog &other );
    void operator=( const ErrorLog &other );
};

class DebugViewErrorLog : public ErrorLog
{
public:
    virtual void write( const std::string &msg );
};

}

#endif // !defined(ERRORLOG_H)


