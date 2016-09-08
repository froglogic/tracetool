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

#include "getcurrentthreadid.h"
#include "timehelper.h" // for now()

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <assert.h>

TRACELIB_NAMESPACE_BEGIN

uint64_t getCurrentProcessStartTime()
{
    // ### BUG: starts counting as of first call only
    static uint64_t t0 = now();
    return t0;
}

ProcessId getCurrentProcessId()
{
    return (ProcessId)::getpid();
}

ThreadId getCurrentThreadId()
{
    return (ThreadId)::pthread_self();
}

TRACELIB_NAMESPACE_END

