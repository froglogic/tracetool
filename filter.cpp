#include "filter.h"

#include <string>

using namespace Tracelib;
using namespace std;

VerbosityFilter::VerbosityFilter()
    : m_maxVerbosity( 1 )
{
}

void VerbosityFilter::setMaximumVerbosity( unsigned short verbosity )
{
    m_maxVerbosity = verbosity;
}

bool VerbosityFilter::acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName )
{
    return verbosity <= m_maxVerbosity;
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

bool PathFilter::acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName )
{
    return startsWith( sourceFile, m_path ); // XXX Implement regex matching
}

void ConjunctionFilter::addFilter( Filter *filter )
{
    m_filters.push_back( filter );
}

bool ConjunctionFilter::acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName )
{
    vector<Filter *>::const_iterator it, end = m_filters.end();
    for ( it = m_filters.begin(); it != end; ++it ) {
        if ( !( *it )->acceptsEntry( verbosity, sourceFile, lineno, functionName ) ) {
            return false;
        }
    }
    return true;
}

void DisjunctionFilter::addFilter( Filter *filter )
{
    m_filters.push_back( filter );
}

bool DisjunctionFilter::acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName )
{
    vector<Filter *>::const_iterator it, end = m_filters.end();
    for ( it = m_filters.begin(); it != end; ++it ) {
        if ( ( *it )->acceptsEntry( verbosity, sourceFile, lineno, functionName ) ) {
            return true;
        }
    }
    return false;
}

