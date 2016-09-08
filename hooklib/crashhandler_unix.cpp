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

#include "crashhandler.h"

#include <cassert>

#include <signal.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

static const int g_caughtSignals[] = {
    SIGSEGV
    , SIGABRT
    , SIGILL
    , SIGFPE
#if defined(SIGBUS)
    , SIGBUS
#endif
};

static TRACELIB_NAMESPACE_IDENT(CrashHandler) g_handler;

extern "C"
{

static void crashHandler( int sig )
{
    for ( unsigned int i = 0; i < sizeof( g_caughtSignals ) / sizeof( g_caughtSignals[0] ); ++i ) {
        signal( g_caughtSignals[i], SIG_DFL );
    }
    (*g_handler)();
}

}

TRACELIB_NAMESPACE_BEGIN

void installCrashHandler( CrashHandler handler )
{
    static bool crashHandlerInstalled = false;

    const char* value = getenv( "TRACELIB_NO_SIGNALHANDLERS" );
    if ( value && strcmp( value, "0" ) )
        return;

    if ( !crashHandlerInstalled ) {
        crashHandlerInstalled = true;
        g_handler = handler;
        for ( unsigned int i = 0; i < sizeof( g_caughtSignals ) / sizeof( g_caughtSignals[0] ); ++i ) {
            signal( g_caughtSignals[i], crashHandler );
        }
    }
}

TRACELIB_NAMESPACE_END
