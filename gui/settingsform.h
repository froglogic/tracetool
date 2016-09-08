/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

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

