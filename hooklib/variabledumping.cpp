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

#include "variabledumping.h"

#include <cassert>
#include <cstdlib> // for free
#include <cstring> // for strncpy, strdup

using namespace std;

TRACELIB_NAMESPACE_BEGIN

VariableValue VariableValue::stringValue( const char *s )
{
    VariableValue var;
    var.m_type = VariableType::String;
    var.m_primitiveValue.string = strdup( s );
    return var;
}

VariableValue VariableValue::numberValue( vlonglong v )
{
    VariableValue var;
    var.m_type = VariableType::Number;
    var.m_primitiveValue.number = v;
    var.m_isSignedNumber = true;
    return var;
}

VariableValue VariableValue::numberValue( vulonglong v )
{
    VariableValue var;
    var.m_type = VariableType::Number;
    var.m_primitiveValue.number = v;
    var.m_isSignedNumber = false;
    return var;
}

VariableValue VariableValue::booleanValue( bool v )
{
    VariableValue var;
    var.m_type = VariableType::Boolean;
    var.m_primitiveValue.boolean = v;
    return var;
}

VariableValue VariableValue::floatValue( long double v )
{
    VariableValue var;
    var.m_type = VariableType::Float;
    var.m_primitiveValue.float_ = v;
    return var;
}

std::string stringRep( const VariableValue &v )
{
    // XXX The list of variable types is duplicated in variabletypes.def
    ostringstream stream;
    switch ( v.type() ) {
        case VariableType::String:
            return v.asString();
        case VariableType::Number:
            if( v.isSignedNumber() ) {
                stream << static_cast<vlonglong>( v.asNumber() );
            } else {
                stream << v.asNumber();
            }
            return stream.str();
        case VariableType::Float:
            stream << v.asFloat();
            return stream.str();
        case VariableType::Boolean:
            return v.asBoolean() ? "true" : "false";
        case VariableType::Unknown:
            assert( !"stringRep on Unknown VariableType" );
    }
    assert( !"Unreachable" );
    return string();
}

size_t VariableValue::convertToString( const VariableValue &v, char *buf, size_t bufsize )
{
    const std::string s = stringRep( v );
    if ( bufsize == 0 ) {
        return s.size() + 1;
    }
    strncpy( buf, s.c_str(), bufsize );
    return bufsize;
}

VariableValue::VariableValue( const VariableValue &other )
    : m_type( other.m_type ),
    m_primitiveValue( other.m_primitiveValue ),
	m_isSignedNumber( other.m_isSignedNumber )
{
    if ( m_type == VariableType::String ) {
        m_primitiveValue.string = strdup( other.asString() );
    }
}

VariableValue::~VariableValue()
{
    if ( m_type == VariableType::String ) {
        free( m_primitiveValue.string );
    }
}

VariableType::Value VariableValue::type() const
{
    return m_type;
}

const char *VariableValue::asString() const
{
    return m_primitiveValue.string;
}

vulonglong VariableValue::asNumber() const
{
    return m_primitiveValue.number;
}

bool VariableValue::asBoolean() const
{
    return m_primitiveValue.boolean;
}

long double VariableValue::asFloat() const
{
    return m_primitiveValue.float_;
}

bool VariableValue::isSignedNumber() const
{
    return m_isSignedNumber;
}

VariableValue::VariableValue()
    : m_type( VariableType::Unknown )
{
}

VariableSnapshot::VariableSnapshot()
    : m_variables( new std::vector<AbstractVariable *> )
{
}

VariableSnapshot::~VariableSnapshot()
{
    delete m_variables;
}


VariableSnapshot &VariableSnapshot::operator<<( AbstractVariable *v )
{
    m_variables->push_back( v );
    return *this;
}

size_t VariableSnapshot::size() const
{
    return m_variables->size();
}


AbstractVariable *&VariableSnapshot::operator[]( size_t idx )
{
    return (*m_variables)[idx];
}

TRACELIB_NAMESPACE_END

