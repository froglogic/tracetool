/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef ACME_H
#define ACME_H

#include "person.h"

#include <string>

#include "tracelib.h"

namespace ACME
{
    
inline std::string f(int v1, int v2)
{
    fDebug(0);
    fWatch(0) << "f() arguments" << fVar(v1) << fVar(v2);
    Person p("John", "Doe");
    return p.toString();
}

namespace GUI
{

class Widget
{
public:
    void repaint(bool onlyVisible = true);
};

}

}

#endif
