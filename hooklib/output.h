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

#ifndef TRACELIB_OUTPUT_H
#define TRACELIB_OUTPUT_H

#include "tracelib_config.h"

#include <stdio.h>
#include <string>
#include <vector>

TRACELIB_NAMESPACE_BEGIN

class Log;
class NetworkOutputPrivate;

class Output
{
public:
    virtual ~Output();

    virtual bool open() { return true; }
    virtual bool canWrite() const { return true; }
    virtual void write( const std::vector<char> &data ) = 0;

protected:
    Output();

private:
    Output( const Output &rhs );
    void operator=( const Output &other );
};

class StdoutOutput : public Output
{
public:
    virtual void write( const std::vector<char> &data );
};

class FileOutput : public Output
{
    std::string m_filename;
    FILE* m_file;
    Log *m_log;
public:
    FileOutput( Log *erroLog, const std::string& filename );
    virtual ~FileOutput();
    virtual void write( const std::vector<char> &data );
    virtual bool open();
    virtual bool canWrite() const;
};

class MultiplexingOutput : public Output
{
public:
    virtual ~MultiplexingOutput();

    void addOutput( Output *output );

    virtual void write( const std::vector<char> &data );

private:
    std::vector<Output *> m_outputs;
};

class NetworkOutput : public Output
{
    std::string m_host;
    unsigned short m_port;
    int m_socket;
    Log *m_log;
    NetworkOutputPrivate *d;
    bool m_lastConnectionAttemptFailed;

    void close();

public:
    NetworkOutput( Log *log, const std::string &remoteHost, unsigned short remotePort );
    virtual ~NetworkOutput();

    virtual bool open();
    virtual bool canWrite() const;
    virtual void write( const std::vector<char> &data );
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_OUTPUT_H)

