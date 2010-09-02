/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef RESTORABLEOBJECT_H
#define RESTORABLEOBJECT_H

#include <QVariant>

class RestorableObject
{
public:
    virtual ~RestorableObject() { }

    virtual QVariant sessionState() const = 0;
    virtual bool restoreSessionState(const QVariant &state) = 0;
};

#endif
