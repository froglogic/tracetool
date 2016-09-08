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

#ifndef ACME_H
#define ACME_H

#include "person.h"

#include <string>

#include "tracelib.h"

namespace ACME
{
    
inline std::string f(int v1, int v2)
{
    fDebug(0) << fEndTrace;
    fWatch(0) << "f() arguments" << fVar(v1) << fVar(v2) << fEndTrace;
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
