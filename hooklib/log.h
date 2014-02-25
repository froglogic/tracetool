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
#include <set>

#ifdef _WIN32
#  include <windows.h>
#  define vsnprintf _vsnprintf
#endif

TRACELIB_NAMESPACE_BEGIN

class LogOutput
{
public:
    virtual ~LogOutput();

    virtual void write( const std::string &msg ) = 0;
protected:
    LogOutput();

private:
    LogOutput( const LogOutput &other );
    void operator=( const LogOutput &other );
};

class Log
{
public:
    Log( LogOutput* statusOutput, LogOutput* errorOutput );
    ~Log();

    void writeError( const std::string &msg );
    inline void writeError( const char *format, ... ) {
        va_list argList;
        va_start( argList, format );

        std::vector<char> buf( 1024, '\0' );
        vsnprintf( &buf[0], buf.size() - 1, format, argList );
        va_end( argList );
        writeError( std::string( &buf[0] ) );
    }

    void writeStatus( const std::string &msg );
    inline void writeStatus( const char *format, ... ) {
        va_list argList;
        va_start( argList, format );

        std::vector<char> buf( 1024, '\0' );
        vsnprintf( &buf[0], buf.size() - 1, format, argList );
        va_end( argList );
        writeStatus( std::string( &buf[0] ) );
    }

private:
    Log( const Log &other );
    void operator=( const Log &other );
    LogOutput* m_statusOutput;
    LogOutput* m_errorOutput;
};

#ifdef _WIN32
class DebugViewLogOutput : public LogOutput
{
public:
    virtual void write( const std::string &msg );
};
#endif

class NullLogOutput : public LogOutput
{
public:
    virtual void write( const std::string &msg );
};

class StreamLogOutput : public LogOutput
{
public:
    explicit StreamLogOutput( std::ostream *stream );
    virtual ~StreamLogOutput();

    virtual void write( const std::string &msg );

private:
    std::ostream *m_stream;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_ERRORLOG_H)


