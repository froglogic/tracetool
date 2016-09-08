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

#include "mutex.h"

#include <pthread.h>

TRACELIB_NAMESPACE_BEGIN

struct MutexHandle {
    pthread_mutex_t mutex;
};

Mutex::Mutex() : m_handle( new MutexHandle )
{
    pthread_mutex_init( &m_handle->mutex, NULL );
}

Mutex::~Mutex()
{
    pthread_mutex_destroy( &m_handle->mutex );
    delete m_handle;
}

void Mutex::lock()
{
    pthread_mutex_lock( &m_handle->mutex );
}

void Mutex::unlock()
{
    pthread_mutex_unlock( &m_handle->mutex );
}

TRACELIB_NAMESPACE_END

