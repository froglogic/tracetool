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
    m_settings->setServerPort(serverPort->value());
    m_settings->setServerOutputFile(QDir::fromNativeSeparators(traceFileEdit->text()));
}

void StorageView::restoreSettings()
{
    // limits
    softLimitSpin->setValue(m_settings->softLimit());
    hardLimitSpin->setValue(m_settings->hardLimit());

    // current file
    QFileInfo fi(m_settings->databaseFile()); // ### let Settings do that
    if (fi.exists()) {
        currentFile->setText(QDir::toNativeSeparators(fi.absoluteFilePath()));
        // ### note life
        currentSize->setText(QString("%1 Bytes").arg(fi.size()));
    } else {
        currentFile->setText(fi.fileName());
    }

    // server
    if (m_settings->startServerAutomatically()) {
        startServerAutomaticallyButton->setChecked(true);
    } else {
        connectToServerButton->setChecked(true);
    }
    serverPort->setValue(m_settings->serverPort());
    traceFileEdit->setText(QDir::toNativeSeparators(m_settings->serverOutputFile()));
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

