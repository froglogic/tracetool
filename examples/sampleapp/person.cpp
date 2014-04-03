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
    fTrace(0) << fEndTrace;
}

std::string Person::toString() const {
    fTrace(0) << "Person -> String conversion" << fEndTrace;
    fWatch(0) << "Person member variables"
                       << fVar(this)
                       << fVar(m_firstName)
                       << fVar(m_lastName)
                       << fEndTrace;
    return m_lastName + ", " + m_firstName;
}
