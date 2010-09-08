/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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
#    define TRACELIB_EXPORT
#  else
#    error "Unsupported compiler!"
#  endif
#endif

#endif // !defined(TRACELIB_DLLDEFS_H)

