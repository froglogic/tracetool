#include "tracelib.h"
#include "serializer.h"
#include "output.h"
#include "filter.h"

#include <string>
#include <sstream>

class Person
{
public:
    Person( const std::string &firstName, const std::string &lastName )
        : m_firstName( firstName ),
        m_lastName( lastName )
    {
        TRACELIB_TRACE
    }

    std::string toString() const {
        TRACELIB_TRACE_MSG("Person -> String conversion")
        TRACELIB_WATCH_MSG("Person member variables",
                       TRACELIB_VAR(this)
                       << TRACELIB_VAR(m_firstName)
                       << TRACELIB_VAR(m_lastName))
        return m_lastName + ", " + m_firstName;
    }

private:
    std::string m_firstName;
    std::string m_lastName;
};

namespace ACME
{

std::string f( int v1, int v2 )
{
    TRACELIB_DEBUG
    TRACELIB_WATCH_MSG("f() arguments", TRACELIB_VAR(v1) << TRACELIB_VAR(v2))
    Person p( "John", "Doe" );
    return p.toString();
}

namespace GUI
{

class Widget
{
public:
    void repaint( bool onlyVisible = true ) {
        if ( !onlyVisible )
            TRACELIB_ERROR
        f( 1313, -2 );
    }
};

}

}

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

