/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "filterform.h"

#include "entryfilter.h"
#include "../hooklib/tracelib.h"

FilterForm::FilterForm(Settings *settings, QWidget *parent)
    : QWidget(parent),
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

    // protect against wrong input
    pidEdit->setValidator(new QIntValidator(this));
    tidEdit->setValidator(new QIntValidator(this));

    connect(applyButton, SIGNAL(clicked()),
            this, SLOT(apply()));
}

void FilterForm::apply()
{
    saveSettings();
    emit filterApplied();
}

void FilterForm::saveSettings()
{
    EntryFilter *f = m_settings->entryFilter();
    f->setApplication(appEdit->text());
    bool ok;
    int pid = pidEdit->text().toInt(&ok);
    f->setProcessId(ok ? pid : -1);
    int tid = tidEdit->text().toInt(&ok);
    f->setThreadId(ok ? tid : -1);
    f->setFunction(funcEdit->text());
    f->setMessage(messageEdit->text());
    f->setType(typeCombo->itemData(typeCombo->currentIndex()).toInt());

    f->emitChanged();
}

void FilterForm::restoreSettings()
{
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
}
