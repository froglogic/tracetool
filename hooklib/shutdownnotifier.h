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

#ifndef TRACELIB_SHUTDOWNNOTIFIER_H
#define TRACELIB_SHUTDOWNNOTIFIER_H

#include "tracelib_config.h"

#include <list>

TRACELIB_NAMESPACE_BEGIN

class ShutdownNotifierObserver
{
public:
    virtual ~ShutdownNotifierObserver();

    virtual void handleProcessShutdown() = 0;
};

class ShutdownNotifier
{
    struct GlobalObject;
    friend struct GlobalObject;

public:
    static ShutdownNotifier &self();

    void addObserver( ShutdownNotifierObserver *observer );
    void removeObserver( ShutdownNotifierObserver *observer );

private:
    ShutdownNotifier();
    ShutdownNotifier( const ShutdownNotifier &other ); // disabled
    void operator=( const ShutdownNotifier &rhs ); // disabled
    ~ShutdownNotifier();

    void notifyObservers();
    static void notifyShutdownObservers();

    std::list<ShutdownNotifierObserver *> m_observers;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_SHUTDOWNNOTIFIER_H)
