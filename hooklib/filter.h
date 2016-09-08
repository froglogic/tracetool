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

#ifndef TRACELIB_FILTER_H
#define TRACELIB_FILTER_H

#include "tracelib_config.h"

#include <string>
#include <vector>

namespace pcrecpp {
    class RE;
}

TRACELIB_NAMESPACE_BEGIN

struct TracePoint;

class Filter
{
public:
    virtual ~Filter();

    virtual bool acceptsTracePoint( const TracePoint *tracePoint ) = 0;

protected:
    Filter();

private:
    Filter( const Filter &rhs );
    void operator=( const Filter &other );
};

enum MatchingMode {
    StrictMatch,
    RegExpMatch,
    WildcardMatch
};

class PathFilter : public Filter
{
public:
    PathFilter();
    virtual ~PathFilter();

    void setPath( MatchingMode matchingMode, const std::string &path );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    MatchingMode m_matchingMode;
    std::string m_path;
    pcrecpp::RE *m_rx;
};

class FunctionFilter : public Filter
{
public:
    FunctionFilter();
    virtual ~FunctionFilter();

    void setFunction( MatchingMode matchingMode, const std::string &function );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    MatchingMode m_matchingMode;
    std::string m_function;
    pcrecpp::RE *m_rx;
};

class GroupFilter : public Filter
{
public:
    enum Mode {
        Blacklist, Whitelist
    };

    GroupFilter();

    void setMode( Mode mode );
    void addGroupName( const std::string &group );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    Mode m_mode;
    std::vector<std::string> m_groups;
};

class ConjunctionFilter : public Filter
{
public:
    virtual ~ConjunctionFilter();

    void addFilter( Filter *filter );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    std::vector<Filter *> m_filters;
};

class DisjunctionFilter : public Filter
{
public:
    virtual ~DisjunctionFilter();

    void addFilter( Filter *filter );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    std::vector<Filter *> m_filters;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_FILTER_H)

