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
        TRACELIB_BEACON(1)
    }

    std::string toString() const {
        TRACELIB_BEACON(1)
        TRACELIB_SNAPSHOT(1) << TRACELIB_VAR(this)
                             << TRACELIB_VAR(m_firstName)
                             << TRACELIB_VAR(m_lastName);
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
    TRACELIB_BEACON(1)
    TRACELIB_SNAPSHOT(1) << TRACELIB_VAR(v1) << TRACELIB_VAR(v2);
    Person p( "John", "Doe" );
    return p.toString();
}

namespace GUI
{

class Widget
{
public:
    void repaint( bool onlyVisible = true ) {
        TRACELIB_BEACON(1)
        f( 1313, -2 );
    }
};

}

}

namespace Tracelib
{
    template <>
    std::string convertVariable<int>( int i ) {
        std::ostringstream str;
        str << i;
        return str.str();
    }

    template <>
    std::string convertVariable<std::string>( std::string s ) {
        return s;
    }

    template <>
    std::string convertVariable<const Person *>( const Person *p ) {
        char buf[ 32 ];
        sprintf( buf, "0x%08x", p );
        return &buf[0];
    }
}

int main()
{
    using namespace Tracelib;
    Trace *trace = new Trace;
    trace->setSerializer( new PlaintextSerializer );
    trace->setOutput( new StdoutOutput );
    setActiveTrace( trace );
    TRACELIB_BEACON(1)
    ACME::GUI::Widget w;
    w.repaint( false );
    delete trace;
}

