/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_CRASHHANDLER_H
#define TRACELIB_CRASHHANDLER_H

#include "tracelib_config.h"

TRACELIB_NAMESPACE_BEGIN

typedef void ( *CrashHandler )();
void installCrashHandler( CrashHandler handler );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_CRASHHANDLER_H)

