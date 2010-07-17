#include "filter.h"

#include "3rdparty/wildcmp/wildcmp.h"

#include <string>

#include <assert.h>

using namespace Tracelib;
using namespace std;

template <class Iterator>
void deleteRange( Iterator begin, Iterator end )
{
    while ( begin != end ) delete *begin++;
}

VerbosityFilter::VerbosityFilter()
    : m_maxVerbosity( 1 )
{
}

void VerbosityFilter::setMaximumVerbosity( unsigned short verbosity )
{
    m_maxVerbosity = verbosity;
}

bool VerbosityFilter::acceptsTracePoint( const TracePoint *tracePoint )
{
    return tracePoint->verbosity <= m_maxVerbosity;
}

static bool startsWith( const string &a, const string &b )
{
    return b.size() <= a.size() && a.substr( 0, b.size() ) == b;
}

PathFilter::PathFilter()
{
}

void PathFilter::setPath( MatchingMode matchingMode, const string &path )
{
    m_matchingMode = matchingMode;
    m_path = path; // XXX Consider normalizing path
}

bool PathFilter::acceptsTracePoint( const TracePoint *tracePoint )
{
    switch ( m_matchingMode ) {
        case StrictMatch:
#ifdef _WIN32
            return _stricmp( tracePoint->sourceFile, m_path.c_str() ) == 0;
#else
            return strcmp( tracePoint->sourceFile, m_path.c_str() ) == 0;
#endif
        case RegExpMatch:
            assert( !"Not implemented" );
            return false;
        case WildcardMatch:
#ifdef _WIN32
            return wildicmp( tracePoint->sourceFile, m_path.c_str() ) != 0;
#else
            return wildcmp( tracePoint->sourceFile, m_path.c_str() ) != 0;
#endif
            return false;
        }
    assert( !"Unreachable" );
    return false;
}

FunctionFilter::FunctionFilter()
{
}

void FunctionFilter::setFunction( MatchingMode matchingMode, const string &function )
{
    m_matchingMode = matchingMode;
    m_function = function;
}

bool FunctionFilter::acceptsTracePoint( const TracePoint *tracePoint )
{
    switch ( m_matchingMode ) {
        case StrictMatch:
            return m_function == tracePoint->functionName;
        case RegExpMatch:
            assert( !"Not implemented" );
            return false;
        case WildcardMatch:
            return wildcmp( tracePoint->functionName, m_function.c_str() ) != 0;
    }
    assert( !"Unreachable" );
    return false;
}

ConjunctionFilter::~ConjunctionFilter()
{
    deleteRange( m_filters.begin(), m_filters.end() );
}

void ConjunctionFilter::addFilter( Filter *filter )
{
    m_filters.push_back( filter );
}

bool ConjunctionFilter::acceptsTracePoint( const TracePoint *tracePoint )
{
    vector<Filter *>::const_iterator it, end = m_filters.end();
    for ( it = m_filters.begin(); it != end; ++it ) {
        if ( !( *it )->acceptsTracePoint( tracePoint ) ) {
            return false;
        }
    }
    return true;
}

DisjunctionFilter::~DisjunctionFilter()
{
    deleteRange( m_filters.begin(), m_filters.end() );
}

void DisjunctionFilter::addFilter( Filter *filter )
{
    m_filters.push_back( filter );
}

bool DisjunctionFilter::acceptsTracePoint( const TracePoint *tracePoint )
{
    vector<Filter *>::const_iterator it, end = m_filters.end();
    for ( it = m_filters.begin(); it != end; ++it ) {
        if ( ( *it )->acceptsTracePoint( tracePoint ) ) {
            return true;
        }
    }
    return false;
}

