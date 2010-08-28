#ifndef ACME_H
#define ACME_H

#include "person.h"

#include <string>

#include "tracelib.h"

namespace ACME
{
    
inline std::string f(int v1, int v2)
{
    TRACELIB_DEBUG;
    TRACELIB_WATCH_MSG("f() arguments", TRACELIB_VAR(v1) << TRACELIB_VAR(v2));
    Person p("John", "Doe");
    return p.toString();
}

namespace GUI
{

class Widget
{
public:
    void repaint(bool onlyVisible = true);
};

}

}

#endif
