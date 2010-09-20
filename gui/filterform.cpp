/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "filterform.h"

#include "entryfilter.h"
#include "../hooklib/tracelib.h"

#include <QSet>

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

void FilterForm::setTraceKeys(const QStringList &keys)
{
    QSet<QString> keysToBeAdded = QSet<QString>::fromList(keys);

    QMap<QString, QListWidgetItem *> currentItems;
    QStringList keysToBeRemoved;
    {
        const int cnt = traceKeyList->count();
        for (int i = 0; i < cnt; ++i) {
            QListWidgetItem *item = traceKeyList->item(i);
            currentItems[item->text()] = item;
            keysToBeRemoved.append(item->text());
        }
    }

    {
        QStringList::Iterator it, end = keysToBeRemoved.end();
        for (it = keysToBeRemoved.begin(); it != end; ++it) {
            QSet<QString>::Iterator keyIt = keysToBeAdded.find(*it);
            if (keyIt != keysToBeAdded.end()) {
                keysToBeRemoved.erase(it);
                keysToBeAdded.erase(keyIt);
            }
        }
    }

    traceKeyList->setUpdatesEnabled(false);
    {
        QStringList::ConstIterator it, end = keysToBeRemoved.end();
        for (it = keysToBeRemoved.begin(); it != end; ++it) {
            delete currentItems[*it];
        }
    }
    {
        QSet<QString>::ConstIterator it, end = keysToBeAdded.end();
        for (it = keysToBeAdded.begin(); it != end; ++it) {
            QListWidgetItem *item = new QListWidgetItem(*it, traceKeyList);
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
        }
    }
    traceKeyList->setUpdatesEnabled(true);
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

    QStringList keys;
    const int cnt = traceKeyList->count();
    for (int i = 0; i < cnt; ++i) {
        QListWidgetItem *item = traceKeyList->item(i);
        if (item->checkState() == Qt::Checked) {
            keys.append(item->text());
        }
    }
    f->setAcceptableKeys(keys);
    f->setAcceptEntriesWithoutKey(showTraceEntriesWithoutKey->checkState() == Qt::Checked);

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
