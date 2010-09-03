/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "settings.h"

class EntryItemModel;
class Server;
class WatchTree;

class MainWindow : public QMainWindow, private Ui::MainWindow,
                   public RestorableObject
{
    Q_OBJECT
public:
    explicit MainWindow(Settings *settings,
                        QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MainWindow();

    bool setDatabase(const QString &databaseFileName, QString *errMsg);

protected:
    // from RestorableObject interface
    QVariant sessionState() const;
    bool restoreSessionState(const QVariant &state);

private slots:
    void fileOpenTrace();
    void helpAbout();
    void toggleFreezeState();
    void openFilterForm();
    void viewShowColumnsDialog();
    void updateColumns();

private:
    void showError(const QString &title, const QString &message);

    Settings* const m_settings;
    EntryItemModel* m_entryItemModel;
    WatchTree* m_watchTree;
    Server *m_server;
};

#endif

