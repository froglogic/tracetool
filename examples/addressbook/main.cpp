/*
    Copyright (c) 2009-10 froglogic GmbH. All rights reserved.

    This file is part of an example program for Squish---it may be used,
    distributed, and modified, without limitation.

    Note that the icons used are from KDE (www.kde.org) and subject to
    the KDE's license.
*/

#include "mainwindow.h"
#include <QtGui/QApplication>
#ifdef Q_WS_X11
#include <QtGui/QWindowsStyle>
#endif
#ifdef Q_OS_AIX
#include "../../../include/qtbuiltinhook.h"
#endif

int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
    QApplication::setStyle(new QWindowsStyle); // Avoid Gtk problems
#endif
    QApplication app(argc, argv);
#ifdef Q_OS_AIX
    Squish::installBuiltinHook(); // On AIX we must manually install a hook
#endif
    MainWindow mainWindow;
    mainWindow.resize(640, 480);
    mainWindow.show();
    return app.exec();
}
