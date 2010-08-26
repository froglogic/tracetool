#include "shutdownnotifier.h"

#include <algorithm>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

const struct ShutdownNotifier::GlobalObject {
    ~GlobalObject() {
        ShutdownNotifier::self().notifyObservers();
    }
} g_globalObject;

ShutdownNotifierObserver::~ShutdownNotifierObserver()
{
}

ShutdownNotifier &ShutdownNotifier::self()
{
    static ShutdownNotifier *instance = 0;
    if ( !instance ) {
        instance = new ShutdownNotifier;
    }
    return *instance;
}

void ShutdownNotifier::addObserver( ShutdownNotifierObserver *observer )
{
    m_observers.push_back( observer );
}

void ShutdownNotifier::removeObserver( ShutdownNotifierObserver *observer )
{
    list<ShutdownNotifierObserver *>::iterator it = find( m_observers.begin(), m_observers.end(), observer );
    if ( it != m_observers.end() ) {
        m_observers.erase( it );
    }
}

ShutdownNotifier::ShutdownNotifier()
{
}

ShutdownNotifier::~ShutdownNotifier()
{
}

void ShutdownNotifier::notifyObservers()
{
    list<ShutdownNotifierObserver *>::iterator it, end = m_observers.end();
    for ( it = m_observers.begin(); it != end; ++it ) {
        ( *it )->handleProcessShutdown();
    }
}

TRACELIB_NAMESPACE_END
