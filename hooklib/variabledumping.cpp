/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "variabledumping.h"

#include <cassert>
#include <sstream>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

VariableValue VariableValue::stringValue( const string &s )
{
    VariableValue var;
    var.m_type = VariableType::String;
    var.m_string = s;
    return var;
}

VariableValue VariableValue::numberValue( unsigned long v )
{
    VariableValue var;
    var.m_type = VariableType::Number;
    var.m_primitiveValue.number = v;
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

string VariableValue::convertToString( const VariableValue &v )
{
    // XXX The list of variable types is duplicated in variabletypes.def
    ostringstream stream;
    switch ( v.type() ) {
        case VariableType::String:
            return v.asString();
        case VariableType::Number:
            stream << v.asNumber();
            return stream.str();
        case VariableType::Float:
            stream << v.asFloat();
            return stream.str();
        case VariableType::Boolean:
            return v.asBoolean() ? "true" : "false";
        case VariableType::Unknown:
            assert( !"convertToString on Unknown VariableType" );
    }
    assert( !"Unreachable" );
    return string();
}

VariableType::Value VariableValue::type() const
{
    return m_type;
}

const string &VariableValue::asString() const
{
    return m_string;
}

unsigned long VariableValue::asNumber() const
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

VariableValue::VariableValue()
    : m_type( VariableType::Unknown )
{
}

VariableSnapshot &operator<<( VariableSnapshot &snapshot, AbstractVariable *v )
{
    snapshot.push_back( v );
    return snapshot;
}

#define TRACELIB_SPECIALIZE_CONVERSION(T, factoryFn) \
template <> \
VariableValue convertVariable( T val ) { \
    return VariableValue::factoryFn( val ); \
}

template <typename T>
string stringFromStringStream( const T &val ) {
    ostringstream str;
    str << val;
    return str.str();
}

#define TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(T) \
template <> \
VariableValue convertVariable( T val ) { \
    return VariableValue::stringValue( stringFromStringStream( val ) ); \
}

TRACELIB_SPECIALIZE_CONVERSION(bool, booleanValue)
TRACELIB_SPECIALIZE_CONVERSION(short, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(unsigned short, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(int, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(unsigned int, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(long, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(unsigned long, numberValue)
TRACELIB_SPECIALIZE_CONVERSION(float, floatValue)
TRACELIB_SPECIALIZE_CONVERSION(double, floatValue)
TRACELIB_SPECIALIZE_CONVERSION(long double, floatValue)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(void *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const void *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(signed char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned char)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(signed char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(unsigned char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const signed char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(const unsigned char *)
TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM(std::string)

#undef TRACELIB_SPECIALIZE_CONVERSION_USING_SSTREAM

TRACELIB_NAMESPACE_END

