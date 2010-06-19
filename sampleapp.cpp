#include "tracelib.h"
#include <string>

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

int main()
{
    tracelib_trace *trace;
    tracelib_create_trace( &trace );
    tracelib_set_default_trace( trace );
    tracelib_trace_set_entry_serializer( trace, &tracelib_plaintext_serializer, NULL );
    tracelib_trace_set_output_writer( trace, &tracelib_stdout_writer, NULL );
    TRACELIB_BEACON(1)
    ACME::GUI::Widget w;
    w.repaint( false );
    tracelib_destroy_trace( trace );
}

