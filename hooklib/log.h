/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_ERRORLOG_H
#define TRACELIB_ERRORLOG_H

#include "tracelib_config.h"

#include "timehelper.h"

#include <cstdarg>
#include <cstdio>
#include <iosfwd>
#include <string>
#include <vector>

#ifdef _WIN32
#  include <windows.h>
#  define vsnprintf _vsnprintf
#endif

TRACELIB_NAMESPACE_BEGIN

class Log
{
public:
    virtual ~Log();

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
    Log();

private:
    Log( const Log &other );
    void operator=( const Log &other );
};

#ifdef _WIN32
class DebugViewLog : public Log
{
public:
    virtual void write( const std::string &msg );
};
#endif

class NullLog : public Log
{
public:
    virtual void write( const std::string &msg );
};

class StreamLog : public Log
{
public:
    explicit StreamLog( std::ostream *stream );
    virtual ~StreamLog();

    virtual void write( const std::string &msg );

private:
    std::ostream *m_stream;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_ERRORLOG_H)


