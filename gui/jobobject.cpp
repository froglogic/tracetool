/**********************************************************************
** Copyright (C) 2011 froglogic GmbH.
** All rights reserved.
**********************************************************************/
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

