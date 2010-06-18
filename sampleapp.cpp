#include <string>

class Person
{
public:
    Person( const std::string &firstName, const std::string &lastName )
        : m_firstName( firstName ),
        m_lastName( lastName )
    {
    }

    std::string toString() const {
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
    Person p( "John", "Doe" );
    return p.toString();
}

namespace GUI
{

class Widget
{
public:
    void repaint( bool onlyVisible = true ) {
        f( 1313, -2 );
    }
};

}

}

int main()
{
    ACME::GUI::Widget w;
    w.repaint( false );
}

