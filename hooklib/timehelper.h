/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "tracelib_config.h"
#include "config.h" // for uint64_t
#include <string>

TRACELIB_NAMESPACE_BEGIN

uint64_t now();
std::ostream &timeToString( std::ostream&, uint64_t );

TRACELIB_NAMESPACE_END
