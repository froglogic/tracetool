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

#ifndef TRACELIB_SERIALIZER_H
#define TRACELIB_SERIALIZER_H

#include "tracelib_config.h"

#include <string>
#include <vector>

#include "configuration.h" // for StorageConfiguration

TRACELIB_NAMESPACE_BEGIN

struct TraceEntry;
struct ProcessShutdownEvent;
class VariableValue;

class Serializer
{
public:
    virtual ~Serializer();

    virtual std::vector<char> serialize( const TraceEntry &entry ) = 0;
    virtual std::vector<char> serialize( const ProcessShutdownEvent &ev ) = 0;

    virtual void setStorageConfiguration( const StorageConfiguration &cfg ) { }

protected:
    Serializer();

private:
    Serializer( const Serializer &rhs );
    void operator=( const Serializer &other );
};

class PlaintextSerializer : public Serializer
{
public:
    PlaintextSerializer();

    void setTimestampsShown( bool timestamps );
    virtual std::vector<char> serialize( const TraceEntry &entry );
    virtual std::vector<char> serialize( const ProcessShutdownEvent &ev );

private:
    std::string convertVariableValue( const VariableValue &v ) const;

    bool m_showTimestamp;
};

class XMLSerializer : public Serializer
{
public:
    XMLSerializer();

    void setBeautifiedOutput( bool beautifiedOutput );

    virtual std::vector<char> serialize( const TraceEntry &entry );
    virtual std::vector<char> serialize( const ProcessShutdownEvent &ev );

    virtual void setStorageConfiguration( const StorageConfiguration &cfg ) {
        m_cfg = cfg;
    }

private:
    std::string convertVariable( const char *name, const VariableValue &v ) const;

    bool m_beautifiedOutput;
    StorageConfiguration m_cfg;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_SERIALIZER_H)

