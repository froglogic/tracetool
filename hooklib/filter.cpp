/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "filter.h"
#include "tracepoint.h"

#include "3rdparty/wildcmp/wildcmp.h"
#include "3rdparty/pcre-8.10/pcrecpp.h"

#include <assert.h>

using namespace std;

template <class Iterator>
void deleteRange( Iterator begin, Iterator end )
{
    while ( begin != end ) delete *begin++;
}

TRACELIB_NAMESPACE_BEGIN

Filter::Filter()
{
}

Filter::~Filter()
{
}

PathFilter::PathFilter()
    : m_rx( 0 )
{
}

PathFilter::~PathFilter()
{
    delete m_rx;
}

void PathFilter::setPath( MatchingMode matchingMode, const string &path )
{
    m_matchingMode = matchingMode;
    m_path = path; // XXX Consider normalizing path
    delete m_rx;

    // XXX Consider encoding issues ('path' is UTF-8 encoded!)
#ifdef _WIN32
    m_rx = new pcrecpp::RE( m_path.c_str(), pcrecpp::CASELESS() );
#else
    m_rx = new pcrecpp::RE( m_path.c_str() );
#endif
}

// XXX Consider encoding issues
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
            return m_rx->FullMatch( tracePoint->sourceFile );
        case WildcardMatch:
#ifdef _WIN32
            return wildicmp( m_path.c_str(), tracePoint->sourceFile ) != 0;
#else
            return wildcmp( m_path.c_str(), tracePoint->sourceFile ) != 0;
#endif
            return false;
        }
    assert( !"Unreachable" );
    return false;
}

FunctionFilter::FunctionFilter()
    : m_rx( 0 )
{
}

FunctionFilter::~FunctionFilter()
{
    delete m_rx;
}

void FunctionFilter::setFunction( MatchingMode matchingMode, const string &function )
{
    m_matchingMode = matchingMode;
    m_function = function;
    delete m_rx;
    m_rx = new pcrecpp::RE( m_function.c_str() );
}

bool FunctionFilter::acceptsTracePoint( const TracePoint *tracePoint )
{
    switch ( m_matchingMode ) {
        case StrictMatch:
            return m_function == tracePoint->functionName;
        case RegExpMatch:
            return m_rx->FullMatch( tracePoint->functionName );
        case WildcardMatch:
            return wildcmp( m_function.c_str(), tracePoint->functionName ) != 0;
    }
    assert( !"Unreachable" );
    return false;
}

GroupFilter::GroupFilter()
    : m_mode( Blacklist )
{
}

void GroupFilter::setMode( Mode mode )
{
    m_mode = mode;
}

void GroupFilter::addGroupName( const string &group )
{
    m_groups.push_back( group );
}

bool GroupFilter::acceptsTracePoint( const TracePoint *tracePoint )
{
    const string tpGroup = tracePoint->groupName ? tracePoint->groupName
                                                 : "";
    bool result = m_mode == Blacklist;
    vector<string>::const_iterator it, end = m_groups.end();
    for ( it = m_groups.begin(); it != end; ++it ) {
        if ( m_mode == Whitelist && *it == tpGroup ) {
            result = true;
            break;
        }
        if ( m_mode == Blacklist && *it == tpGroup ) {
            result = false;
            break;
        }
    }
    return result;
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

TRACELIB_NAMESPACE_END

