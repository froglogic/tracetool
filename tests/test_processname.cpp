/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "tracelib.h"
#include "configuration.h"

#include <iostream>

using namespace std;

int g_failureCount = 0;
int g_verificationCount = 0;

template <typename T>
static void verify( const char *what, T expected, T actual )
{
    if ( !( expected == actual ) ) {
        cout << "FAIL: " << what << "; expected '" << boolalpha << expected << "', got '" << boolalpha << actual << "'" << endl;
        ++g_failureCount;
    }
    ++g_verificationCount;
}

TRACELIB_NAMESPACE_BEGIN

static void testProcessName()
{
    verify( "Configuration::currentProcessName",
            string( "test_processname" ),
            Configuration::currentProcessName() );
    verify( "Configuration::defaultFileName",
            true,
            Configuration::defaultFileName().rfind( TRACELIB_DEFAULT_CONFIGFILE_NAME ) != string::npos );
}

TRACELIB_NAMESPACE_END

int main()
{
    TRACELIB_NAMESPACE_IDENT(testProcessName)();
    cout << g_verificationCount << " verifications; " << g_failureCount << " failures found." << endl;
    return g_failureCount;
}

