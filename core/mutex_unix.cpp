#include "mutex.h"

#include <cassert>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

Mutex::Mutex() : m_handle( 0 )
{
    assert( !"Mutex operations not implemented on Unix" );
}

Mutex::~Mutex()
{
    assert( !"Mutex operations not implemented on Unix" );
}

void Mutex::lock()
{
    assert( !"Mutex operations not implemented on Unix" );
}

void Mutex::unlock()
{
    assert( !"Mutex operations not implemented on Unix" );
}

TRACELIB_NAMESPACE_END

