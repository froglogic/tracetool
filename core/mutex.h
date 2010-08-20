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

