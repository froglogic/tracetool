#include "variabledumping.h"

#include <sstream>

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

template <typename T>
string stringFromStringStream( const T &val ) {
    ostringstream str;
    str << val;
    return str.str();
}

#define TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(T) \
template <> \
VariableValue convertVariable( T val ) { \
    return VariableValue::stringValue( stringFromStringStream( val ) ); \
}

TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(short)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(unsigned short)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(int)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(unsigned int)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(long)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(unsigned long)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(float)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(double)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(long double)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(const void *)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(char)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(signed char)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(unsigned char)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(const char *)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(const signed char *)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(const unsigned char *)
TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM(std::string)

#undef TRACELIB_SPECIALIZE_CONVERISON_USING_SSTREAM

TRACELIB_NAMESPACE_END
