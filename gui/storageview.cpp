/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "storageview.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

StorageView::StorageView(Settings *settings, QWidget *parent)
    : m_settings(settings)
{
    setupUi(this);

    connect(traceFileBrowseButton, SIGNAL(clicked()), SLOT(browseForTraceFile()));

    restoreSettings();
}

void StorageView::accept()
{
    saveSettings();
    QDialog::accept();
}

void StorageView::saveSettings()
{
    m_settings->setSoftLimit(softLimitSpin->value());
    m_settings->setHardLimit(hardLimitSpin->value());
    m_settings->setStartServerAutomatically(startServerAutomaticallyButton->isChecked());
    m_settings->setServerGUIPort(serverPort->value());
    m_settings->setServerTracePort(serverTracePort->value());
    m_settings->setDatabaseFile(QDir::fromNativeSeparators(traceFileEdit->text()));
}

void StorageView::restoreSettings()
{
    // limits
    softLimitSpin->setValue(m_settings->softLimit());
    hardLimitSpin->setValue(m_settings->hardLimit());

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

