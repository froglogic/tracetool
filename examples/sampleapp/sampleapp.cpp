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

#include "acme.h"
#include "person.h"

#include <tracelib.h>

#include <cstdio>
#include <string>
#include <sstream>

TRACELIB_NAMESPACE_BEGIN
    template <>
    VariableValue convertVariable<const Person *>( const Person *p ) {
        char buf[ 32 ];
        sprintf( buf, "0x%p", static_cast<const void*>(p) );
        return VariableValue::stringValue( &buf[0] );
    }
TRACELIB_NAMESPACE_END

int main()
{
    fTrace(0) << fEndTrace;
    ACME::GUI::Widget w;
    w.repaint( false );
}

