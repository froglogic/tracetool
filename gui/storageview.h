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

#ifndef STORAGEVIEW_H
#define STORAGEVIEW_H

#include <QDialog>
#include "ui_storageview.h"
#include "settings.h"

class StorageView : public QDialog, private Ui::StorageView
{
    Q_OBJECT
public:
    explicit StorageView(Settings *settings, QWidget *parent = 0);

    void accept();

    void restoreSettings();

private slots:
    void browseForTraceFile();

private:
    void saveSettings();

    Settings* const m_settings;
};

#endif

