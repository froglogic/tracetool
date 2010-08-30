/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "person.h"
#include <tracelib.h>

Person::Person(const std::string &firstName, const std::string &lastName)
    : m_firstName(firstName),
      m_lastName(lastName)
{
    TRACELIB_TRACE;
}

std::string Person::toString() const {
    TRACELIB_TRACE_MSG("Person -> String conversion");
    TRACELIB_WATCH_MSG("Person member variables",
                       TRACELIB_VAR(this)
                       << TRACELIB_VAR(m_firstName)
                       << TRACELIB_VAR(m_lastName));
    return m_lastName + ", " + m_firstName;
}
