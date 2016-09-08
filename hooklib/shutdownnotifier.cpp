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
