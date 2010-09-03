/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "columnsform.h"

#include "entryfilter.h"
#include "../core/tracelib.h"

ColumnsForm::ColumnsForm(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_settings(settings)
{
    setupUi(this);

    restoreSettings();
}

void ColumnsForm::accept()
{
    saveSettings();
    QDialog::accept();
}

void ColumnsForm::saveSettings()
{
}

void ColumnsForm::restoreSettings()
{
/*
    EntryFilter *f = m_settings->entryFilter();
    appEdit->setText(f->application());
    if (f->processId() != -1)
        pidEdit->setText(QString::number(f->processId()));
    else
        pidEdit->clear();
    if (f->threadId() != -1)
        tidEdit->setText(QString::number(f->threadId()));
    else
        tidEdit->clear();
    funcEdit->setText(f->function());
    messageEdit->setText(f->message());
    int idx = typeCombo->findData(f->type());
    if (idx != -1)
        typeCombo->setCurrentIndex(idx);
*/
}
