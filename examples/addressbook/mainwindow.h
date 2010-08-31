#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/*
    Copyright (c) 2009-10 froglogic GmbH. All rights reserved.

    This file is part of an example program for Squish---it may be used,
    distributed, and modified, without limitation.

    Note that the icons used are from KDE (www.kde.org) and subject to
    the KDE's license.
*/

#include <QtGui/QMainWindow>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE
class QAction;
class QTableWidget;
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void fileNew();
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void editAdd();
    void editRemove();
    void setDirty(bool on=true);
    void updateUi();

private:
    void clear();
    void load(const QString &name);
    bool okToContinue();

    QAction *fileSaveAction;
    QAction *fileSaveAsAction;
    QMenu *editMenu;
    QAction *editAddAction;
    QAction *editRemoveAction;
    QTableWidget *tableWidget;
    QString filename;
    bool dirty;
};

#endif // MAINWINDOW_H
