#include "acme.h"

namespace ACME
{

namespace GUI
{

    void Widget::repaint(bool onlyVisible) {
        if (!onlyVisible)
            TRACELIB_ERROR;
        f(1313, -2);
    }

}

}
