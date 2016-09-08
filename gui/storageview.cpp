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

#include "storageview.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

StorageView::StorageView(Settings *settings, QWidget *parent)
    : m_settings(settings)
{
    setupUi(this);

    connect(traceFileBrowseButton, SIGNAL(clicked()), SLOT(browseForTraceFile()));

    restoreSettings();
}

void StorageView::accept()
{
    if (startServerAutomaticallyButton->isChecked() &&
        serverPort->value() == serverTracePort->value()) {
        QMessageBox::critical(this,
                tr("Cannot Apply Settings"),
                tr("Cannot apply these settings; the two ports opened by the "
                   "trace server must have different numbers."));
        return;
    }
    saveSettings();
    QDialog::accept();
}

void StorageView::saveSettings()
{
    m_settings->setStartServerAutomatically(startServerAutomaticallyButton->isChecked());
    m_settings->setServerGUIPort(serverPort->value());
    m_settings->setServerTracePort(serverTracePort->value());
    m_settings->setDatabaseFile(QDir::fromNativeSeparators(traceFileEdit->text()));
}

void StorageView::restoreSettings()
{
    // server
    if (m_settings->startServerAutomatically()) {
        startServerAutomaticallyButton->setChecked(true);
    } else {
        connectToServerButton->setChecked(true);
    }
    serverPort->setValue(m_settings->serverGUIPort());
    serverTracePort->setValue(m_settings->serverTracePort());
    traceFileEdit->setText(QDir::toNativeSeparators(m_settings->databaseFile()));
}

void StorageView::browseForTraceFile()
{
    QString fn = QFileDialog::getSaveFileName(
            this,
            tr("Server Output File"),
            QString(),
            tr("Trace Files (*.trace)"));
    if (!fn.isEmpty()) {
        traceFileEdit->setText(QDir::toNativeSeparators(fn));
    }
}

