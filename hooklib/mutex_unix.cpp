/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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

