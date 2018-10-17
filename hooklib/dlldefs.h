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

#ifndef TRACELIB_DLLDEFS_H
#define TRACELIB_DLLDEFS_H

/* Only define TRACELIB_EXPORT if it wasn't defined already. This makes
 * it possible for client code to directly compile tracelib source
 * files into the resulting binary. TRACELIB_EXPORT can be defined to
 * an empty value for this use case.
 */
#ifndef TRACELIB_EXPORT
#  ifdef _MSC_VER
#    ifdef TRACELIB_MAKEDLL
#      define TRACELIB_EXPORT __declspec(dllexport)
#    else
#      define TRACELIB_EXPORT __declspec(dllimport)
#    endif
#  elif __GNUC__
#    if __GNUC__ - 0 > 3
#      define TRACELIB_EXPORT __attribute__ ((visibility("default"))
#    else
#      define TRACELIB_EXPORT
#  else
#    error "Unsupported compiler!"
#  endif
#endif

#endif // !defined(TRACELIB_DLLDEFS_H)

