#include "acme.h"
#include "person.h"

#include "tracelib.h"

#include <cstdio>
#include <string>
#include <sstream>

TRACELIB_NAMESPACE_BEGIN
    template <>
    VariableValue convertVariable<int>( int i ) {
        std::ostringstream str;
        str << i;
        return VariableValue::stringValue( str.str() );
    }

    template <>
    VariableValue convertVariable<std::string>( std::string s ) {
        return VariableValue::stringValue( s );
    }

    template <>
    VariableValue convertVariable<const Person *>( const Person *p ) {
        char buf[ 32 ];
        sprintf( buf, "0x%08x", p );
        return VariableValue::stringValue( &buf[0] );
    }
TRACELIB_NAMESPACE_END

int main()
{
    TRACELIB_TRACE
    ACME::GUI::Widget w;
    w.repaint( false );
}

