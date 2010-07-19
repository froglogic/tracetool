#include "filter.h"

#include <iostream>

using namespace std;
using namespace Tracelib;

int g_failureCount = 0;
int g_verificationCount = 0;

template <typename T>
static void verify( const char *what, T expected, T actual )
{
    if ( !( expected == actual ) ) {
        cout << "FAIL: " << what << "; expected '" << expected << "', got '" << boolalpha << actual << "'" << endl;
        ++g_failureCount;
    }
    ++g_verificationCount;
}

static void testVerbosityFilter()
{
    VerbosityFilter lowVerbosityFilter;
    lowVerbosityFilter.setMaximumVerbosity( 0 );
    VerbosityFilter mediumVerbosityFilter;
    mediumVerbosityFilter.setMaximumVerbosity( 1 );
    VerbosityFilter highVerbosityFilter;
    highVerbosityFilter.setMaximumVerbosity( 2 );

    static TracePoint lowVerbosityTP( 1, NULL, 0, NULL );
    static TracePoint mediumVerbosityTP( 2, NULL, 0, NULL );
    static TracePoint highVerbosityTP( 3, NULL, 0, NULL );

    verify( "lowVerbosityFilter on lowVerbosityTP", false, lowVerbosityFilter.acceptsTracePoint( &lowVerbosityTP ) );
    verify( "lowVerbosityFilter on mediumVerbosityTP", false, lowVerbosityFilter.acceptsTracePoint( &mediumVerbosityTP ) );
    verify( "lowVerbosityFilter on highVerbosityTP", false, lowVerbosityFilter.acceptsTracePoint( &highVerbosityTP ) );

    verify( "mediumVerbosityFilter on lowVerbosityTP", true, mediumVerbosityFilter.acceptsTracePoint( &lowVerbosityTP ) );
    verify( "mediumVerbosityFilter on mediumVerbosityTP", false, mediumVerbosityFilter.acceptsTracePoint( &mediumVerbosityTP ) );
    verify( "mediumVerbosityFilter on highVerbosityTP", false, mediumVerbosityFilter.acceptsTracePoint( &highVerbosityTP ) );

    verify( "highVerbosityFilter on lowVerbosityTP", true, highVerbosityFilter.acceptsTracePoint( &lowVerbosityTP ) );
    verify( "highVerbosityFilter on mediumVerbosityTP", true, highVerbosityFilter.acceptsTracePoint( &mediumVerbosityTP ) );
    verify( "highVerbosityFilter on highVerbosityTP", false, highVerbosityFilter.acceptsTracePoint( &highVerbosityTP ) );
}

int main()
{
    testVerbosityFilter();
    cout << g_verificationCount << " verifications; " << g_failureCount << " failures found." << endl;
    return g_failureCount;
}

