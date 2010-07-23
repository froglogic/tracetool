#include "filter.h"

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

static void testVerbosityFilter()
{
    VerbosityFilter lowVerbosityFilter;
    lowVerbosityFilter.setMaximumVerbosity( 0 );
    VerbosityFilter mediumVerbosityFilter;
    mediumVerbosityFilter.setMaximumVerbosity( 1 );
    VerbosityFilter highVerbosityFilter;
    highVerbosityFilter.setMaximumVerbosity( 2 );

    static TracePoint lowVerbosityTP( TracePoint::LogPoint, 1, NULL, 0, NULL );
    static TracePoint mediumVerbosityTP(TracePoint::LogPoint,  2, NULL, 0, NULL );
    static TracePoint highVerbosityTP( TracePoint::LogPoint, 3, NULL, 0, NULL );

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

static void testStrictPathFilter()
{
    static TracePoint tpLowerCase( TracePoint::LogPoint, 0, "c:\\foo\\bar\\mysrc.cpp", 0, NULL );
    static TracePoint tpUpperCase( TracePoint::LogPoint, 0, "C:\\Foo\\Bar\\mysrc.cpp", 0, NULL );

    PathFilter emptyPathFilter;
    emptyPathFilter.setPath( StrictMatch, "" );
    verify( "emptyPathFilter on tpLowerCase", false, emptyPathFilter.acceptsTracePoint( &tpLowerCase ) );
    verify( "emptyPathFilter on tpUpperCase", false, emptyPathFilter.acceptsTracePoint( &tpUpperCase ) );

    PathFilter otherFileFilter;
    otherFileFilter.setPath( StrictMatch, "C:\\Foo\\Bar\\someotherfile.cpp" );
    verify( "otherFileFilter on tpLowerCase", false, otherFileFilter.acceptsTracePoint( &tpLowerCase ) );
    verify( "otherFileFilter on tpUpperCase", false, otherFileFilter.acceptsTracePoint( &tpUpperCase ) );

    PathFilter samePathLCFilter;
    samePathLCFilter.setPath( StrictMatch, "c:\\foo\\bar\\mysrc.cpp" );
    verify( "samePathLCFilter on tpLowerCase", true, samePathLCFilter.acceptsTracePoint( &tpLowerCase ) );
#ifdef _WIN32
    verify( "samePathLCFilter on tpUpperCase", true, samePathLCFilter.acceptsTracePoint( &tpUpperCase ) );
#else
    verify( "samePathLCFilter on tpUpperCase", false, samePathLCFilter.acceptsTracePoint( &tpUpperCase ) );
#endif

    PathFilter samePathUCFilter;
    samePathUCFilter.setPath( StrictMatch, "C:\\FOO\\BAR\\MYSRC.CPP" );
    verify( "samePathUCFilter on tpLowerCase", true, samePathUCFilter.acceptsTracePoint( &tpLowerCase ) );
#ifdef _WIN32
    verify( "samePathUCFilter on tpUpperCase", true, samePathUCFilter.acceptsTracePoint( &tpUpperCase ) );
#else
    verify( "samePathUCFilter on tpUpperCase", false, samePathUCFilter.acceptsTracePoint( &tpUpperCase ) );
#endif

    PathFilter samePathFilter;
    samePathFilter.setPath( StrictMatch, "C:\\Foo\\Bar\\mysrc.cpp" );
#ifdef _WIN32
    verify( "samePathFilter on tpLowerCase", true, samePathFilter.acceptsTracePoint( &tpLowerCase ) );
#else
    verify( "samePathFilter on tpLowerCase", false, samePathFilter.acceptsTracePoint( &tpLowerCase ) );
#endif
    verify( "samePathFilter on tpUpperCase", true, samePathFilter.acceptsTracePoint( &tpUpperCase ) );
}

static void testWildcardPathFilter()
{
    static TracePoint tpLowerCase( TracePoint::LogPoint, 0, "c:\\foo\\bar\\mysrc.cpp", 0, NULL );
    static TracePoint tpUpperCase( TracePoint::LogPoint, 0, "C:\\Foo\\Bar\\mysrc.cpp", 0, NULL );

    PathFilter emptyPathFilter;
    emptyPathFilter.setPath( WildcardMatch, "" );
    verify( "emptyPathFilter on tpLowerCase", false, emptyPathFilter.acceptsTracePoint( &tpLowerCase ) );
    verify( "emptyPathFilter on tpUpperCase", false, emptyPathFilter.acceptsTracePoint( &tpUpperCase ) );

    PathFilter everyPathFilter;
    everyPathFilter.setPath( WildcardMatch, "*" );
    verify( "everyPathFilter on tpLowerCase", true, everyPathFilter.acceptsTracePoint( &tpLowerCase ) );
    verify( "everyPathFilter on tpUpperCase", true, everyPathFilter.acceptsTracePoint( &tpUpperCase ) );

    PathFilter everyPathFilter2;
    everyPathFilter2.setPath( WildcardMatch, "***" );
    verify( "everyPathFilter2 on tpLowerCase", true, everyPathFilter2.acceptsTracePoint( &tpLowerCase ) );
    verify( "everyPathFilter2 on tpUpperCase", true, everyPathFilter2.acceptsTracePoint( &tpUpperCase ) );

    PathFilter otherFileFilter;
    otherFileFilter.setPath( WildcardMatch, "*\\someotherfile.cpp" );
    verify( "otherFileFilter on tpLowerCase", false, otherFileFilter.acceptsTracePoint( &tpLowerCase ) );
    verify( "otherFileFilter on tpUpperCase", false, otherFileFilter.acceptsTracePoint( &tpUpperCase ) );

    PathFilter samePathLCFilter;
    samePathLCFilter.setPath( WildcardMatch, "*\\mysrc.cpp" );
    verify( "samePathLCFilter on tpLowerCase", true, samePathLCFilter.acceptsTracePoint( &tpLowerCase ) );
    verify( "samePathLCFilter on tpUpperCase", true, samePathLCFilter.acceptsTracePoint( &tpUpperCase ) );

    PathFilter samePathUCFilter;
    samePathUCFilter.setPath( WildcardMatch, "*\\Bar\\mysrc.cpp" );
#ifdef _WIN32
    verify( "samePathUCFilter on tpLowerCase", true, samePathUCFilter.acceptsTracePoint( &tpLowerCase ) );
#else
    verify( "samePathUCFilter on tpLowerCase", false, samePathUCFilter.acceptsTracePoint( &tpLowerCase ) );
#endif
    verify( "samePathUCFilter on tpUpperCase", true, samePathUCFilter.acceptsTracePoint( &tpUpperCase ) );

    PathFilter questionMarkFilter;
    questionMarkFilter.setPath( WildcardMatch, "*\\?ar\\mysrc.cpp" );
    verify( "questionMarkFilter on tpLowerCase", true, questionMarkFilter.acceptsTracePoint( &tpLowerCase ) );
    verify( "questionMarkFilter on tpUpperCase", true, questionMarkFilter.acceptsTracePoint( &tpUpperCase ) );
}

static void testPathFilter()
{
    testStrictPathFilter();
    testWildcardPathFilter();
}

TRACELIB_NAMESPACE_END

int main()
{
    TRACELIB_NAMESPACE_IDENT(testVerbosityFilter)();
    TRACELIB_NAMESPACE_IDENT(testPathFilter)();
    cout << g_verificationCount << " verifications; " << g_failureCount << " failures found." << endl;
    return g_failureCount;
}

