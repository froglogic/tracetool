#ifndef LINEEDIT_H
#define LINEEDIT_H
/*
    Copyright (c) 2009-10 froglogic GmbH. All rights reserved.

    This file is part of an example program for Squish---it may be used,
    distributed, and modified, without limitation.

    Note that the icons used are from KDE (www.kde.org) and subject to
    the KDE's license.
*/


#include <QtGui/QLineEdit>
#include <QtGui/QRegExpValidator>


class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget *parent=0)
        : QLineEdit(parent)
    {
        QRegExp regex("^[^|]*$");
        setValidator(new QRegExpValidator(regex, this));
    }
};

#endif // LINEEDIT_H
