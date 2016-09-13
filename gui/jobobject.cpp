/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2011-2016 froglogic GmbH
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

#include "jobobject.h"

JobObject::JobObject() : m_job( INVALID_HANDLE_VALUE )
{
    m_job = ::CreateJobObject( NULL, NULL );
    if ( m_job ) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = { 0 };
        if ( ::QueryInformationJobObject( m_job, JobObjectExtendedLimitInformation, &info, sizeof( info ), NULL ) ) {
            info.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            ::SetInformationJobObject( m_job, JobObjectExtendedLimitInformation, &info, sizeof( info ) );
        }
    }
}

JobObject::~JobObject()
{
    ::CloseHandle( m_job );
    m_job = INVALID_HANDLE_VALUE;
}

void JobObject::assignProcess( HANDLE process )
{
    (void)AssignProcessToJobObject( m_job, process );
}

