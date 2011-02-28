/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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

