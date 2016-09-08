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

#ifndef TRACELIB_MUTEX_H
#define TRACELIB_MUTEX_H

#include "tracelib_config.h"

TRACELIB_NAMESPACE_BEGIN

struct MutexHandle;

class Mutex
{
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    Mutex( const Mutex &other ); // disabled
    void operator=( const Mutex &rhs ); // disabled

    MutexHandle *m_handle;
};

class MutexLocker
{
public:
    MutexLocker( Mutex &m ) : m_mutex( m ) { m_mutex.lock(); }
    ~MutexLocker() { m_mutex.unlock(); }

private:
    MutexLocker( const MutexLocker &other ); // disabled
    void operator=( const MutexLocker &rhs ); // disabled

    Mutex &m_mutex;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_MUTEX_H)

