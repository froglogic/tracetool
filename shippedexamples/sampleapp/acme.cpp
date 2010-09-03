/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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
