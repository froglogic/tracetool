#include "filter.h"

#include <string>

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

bool VerbosityFilter::acceptsEntry( const TraceEntry &entry )
{
    return entry.verbosity <= m_maxVerbosity;
}

static bool startsWith( const string &a, const string &b )
{
    return b.size() <= a.size() && a.substr( 0, b.size() ) == b;
}

PathFilter::PathFilter()
{
}

void PathFilter::setPath( const string &path )
{
    m_path = path;
}

bool PathFilter::acceptsEntry( const TraceEntry &entry )
{
    return startsWith( entry.sourceFile, m_path ); // XXX Implement regex matching
}

ConjunctionFilter::~ConjunctionFilter()
{
    deleteRange( m_filters.begin(), m_filters.end() );
}

void ConjunctionFilter::addFilter( Filter *filter )
{
    m_filters.push_back( filter );
}

bool ConjunctionFilter::acceptsEntry( const TraceEntry &entry )
{
    vector<Filter *>::const_iterator it, end = m_filters.end();
    for ( it = m_filters.begin(); it != end; ++it ) {
        if ( !( *it )->acceptsEntry( entry ) ) {
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

bool DisjunctionFilter::acceptsEntry( const TraceEntry &entry )
{
    vector<Filter *>::const_iterator it, end = m_filters.end();
    for ( it = m_filters.begin(); it != end; ++it ) {
        if ( ( *it )->acceptsEntry( entry ) ) {
            return true;
        }
    }
    return false;
}

