#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "settings.h"

class EntryItemModel;

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
    void openTrace();

private:
    void showError(const QString &title, const QString &message);

    Settings* const m_settings;
    EntryItemModel* m_model;
};

#endif

