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

#include <windows.h>

TRACELIB_NAMESPACE_BEGIN

struct MutexHandle {
    CRITICAL_SECTION section;
};

Mutex::Mutex() : m_handle( new MutexHandle )
{
    ::InitializeCriticalSection( &m_handle->section );
}

Mutex::~Mutex()
{
    ::DeleteCriticalSection( &m_handle->section );
    delete m_handle;
}

void Mutex::lock()
{
    ::EnterCriticalSection( &m_handle->section );
}

void Mutex::unlock()
{
    ::LeaveCriticalSection( &m_handle->section );
}

TRACELIB_NAMESPACE_END

