/*
    Copyright (c) 2009-18 froglogic GmbH. All rights reserved.

    This file is part of an example program for Squish---it may be used,
    distributed, and modified, without limitation.

    Note that the icons used are from KDE (www.kde.org) and subject to
    the KDE's license.
*/

#include "lineedit.h"
#include <QRegExpValidator>

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    QRegExp regex("^[^|]*$");
    setValidator(new QRegExpValidator(regex, this));
}