/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QDialog>
#include <QFont>
#include "ui_settingsform.h"
#include "settings.h"

class SettingsForm : public QDialog, private Ui::SettingsForm
{
    Q_OBJECT
public:
    explicit SettingsForm(Settings *settings,
                          QWidget *parent = 0, Qt::WindowFlags flags = 0);

protected:
    void accept();

private slots:
    void moveUp();
    void moveDown();
    void moveToInvisible();
    void moveToVisible();
    void selectFont();

private:
    void saveSettings();
    void restoreSettings();

    Settings* const m_settings;
    QFont m_currentFont;
};

#endif

