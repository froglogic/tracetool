/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_DLLDEFS_H
#define TRACELIB_DLLDEFS_H

#ifdef _MSC_VER
#  ifdef TRACELIB_MAKEDLL
#    define TRACELIB_EXPORT __declspec(dllexport)
#  else
#    define TRACELIB_EXPORT __declspec(dllimport)
#  endif
#elif __GNUC__
#  define TRACELIB_EXPORT
#else
#  error "Unsupported compiler!"
#endif

#endif // !defined(TRACELIB_DLLDEFS_H)

