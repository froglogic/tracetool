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

#ifndef TRACELIB_CRASHHANDLER_H
#define TRACELIB_CRASHHANDLER_H

#include "tracelib_config.h"

TRACELIB_NAMESPACE_BEGIN

typedef void ( *CrashHandler )();
void installCrashHandler( CrashHandler handler );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_CRASHHANDLER_H)

