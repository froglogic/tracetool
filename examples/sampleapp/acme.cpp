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
        if (!onlyVisible) {
            fError(0);
        }
        f(1313, -2);
    }

}

}
