#include "variabledumping.h"

using namespace std;

TRACELIB_NAMESPACE_BEGIN

VariableValue VariableValue::stringValue( const string &s )
{
    return VariableValue( s );
}

VariableValue::Type VariableValue::type() const
{
    return m_type;
}

const string &VariableValue::asString() const
{
    return m_string;
}

VariableValue::VariableValue( const string &s )
    : m_type( String ),
    m_string( s )
{
}

VariableSnapshot &operator<<( VariableSnapshot &snapshot, AbstractVariable *v )
{
    snapshot.push_back( v );
    return snapshot;
}

TRACELIB_NAMESPACE_END
