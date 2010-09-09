/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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

    std::list<ShutdownNotifierObserver *> m_observers;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_SHUTDOWNNOTIFIER_H)
