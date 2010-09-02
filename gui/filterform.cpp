/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "filterform.h"

#include "entryfilter.h"
#include "../core/tracelib.h"

FilterForm::FilterForm(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_settings(settings)
{
    setupUi(this);

    // fill type combobox
    typeCombo->addItem("", -1);
    using TRACELIB_NAMESPACE_IDENT(TracePointType);
    const int *types = TracePointType::values();
    int t;
    while ((t = *++types) != -1) {
        TracePointType::Value v = TracePointType::Value(t);
        QString typeName = TracePointType::valueAsString(v);
        typeCombo->addItem(typeName, t);
    }

    restoreSettings();
}

void FilterForm::accept()
{
    saveSettings();
    QDialog::accept();
}

void FilterForm::saveSettings()
{
    EntryFilter *f = m_settings->entryFilter();
    f->setApplication(appEdit->text());
    f->setProcessId(pidEdit->text());
    f->setThreadId(tidEdit->text());
    f->setFunction(funcEdit->text());
    f->setMessage(messageEdit->text());
    f->setType(typeCombo->itemData(typeCombo->currentIndex()).toInt());
}

void FilterForm::restoreSettings()
{
    EntryFilter *f = m_settings->entryFilter();
    appEdit->setText(f->application());
    pidEdit->setText(f->processId());
    tidEdit->setText(f->threadId());
    funcEdit->setText(f->function());
    messageEdit->setText(f->message());
    int idx = typeCombo->findData(f->type());
    if (idx != -1)
        typeCombo->setCurrentIndex(idx);
}
