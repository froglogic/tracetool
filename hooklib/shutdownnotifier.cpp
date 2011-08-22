/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "shutdownnotifier.h"

#include <algorithm>

#include <cstdlib> // for std::atexit

using namespace std;

TRACELIB_NAMESPACE_BEGIN

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
    std::atexit( notifyShutdownObservers );
}

ShutdownNotifier::~ShutdownNotifier()
{
}

void ShutdownNotifier::notifyShutdownObservers()
{
    ShutdownNotifier::self().notifyObservers();
}

void ShutdownNotifier::notifyObservers()
{
    list<ShutdownNotifierObserver *>::iterator it, end = m_observers.end();
    for ( it = m_observers.begin(); it != end; ++it ) {
        ( *it )->handleProcessShutdown();
    }
}

TRACELIB_NAMESPACE_END
