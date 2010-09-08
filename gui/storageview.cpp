/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "storageview.h"

#include <QDir>
#include <QFileInfo>

StorageView::StorageView(Settings *settings, QWidget *parent)
    : m_settings(settings)
{
    setupUi(this);

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
}
