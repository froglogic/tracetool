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

void FilterForm::setTraceKeys( const QStringList &keys )
{
    traceKeyList->clear();
    addTraceKeys(keys);
}

void FilterForm::addTraceKeys( const QStringList &keys )
{
    QSet<QString> currentKeys;
    for ( int i = 0; i < traceKeyList->count(); ++i ) {
        currentKeys.insert( traceKeyList->item( i )->text() );
    }

    QStringList::ConstIterator it, end = keys.end();
    for ( it = keys.begin(); it != end; ++it ) {
        const QString &keyName = *it;
        if ( !currentKeys.contains( keyName ) ) {
            QListWidgetItem *i = new QListWidgetItem( keyName, traceKeyList );
            bool active = traceKeyDefaultState( keyName );
            i->setCheckState(active ? Qt::Checked : Qt::Unchecked);
            currentKeys.insert( keyName );

            // Update filtered (inactive) keys list too.
            EntryFilter *f = m_settings->entryFilter();
            QStringList inactiveKeys = f->inactiveKeys();
            if (!active)
                inactiveKeys.append( keyName );
            else
                inactiveKeys.removeAll( keyName );
            inactiveKeys.removeDuplicates();
            f->setInactiveKeys(inactiveKeys);
            f->emitChanged();
        }
    }
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

    QStringList inactiveKeys;
    for (int i = 0; i < traceKeyList->count(); ++i) {
        QListWidgetItem *item = traceKeyList->item(i);
        if (item->checkState() != Qt::Checked) {
            inactiveKeys.append(item->text());
        }
    }
    f->setInactiveKeys(inactiveKeys);
    f->setAcceptEntriesWithoutKey(acceptEntriesWithoutKey->isChecked());
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

bool FilterForm::traceKeyDefaultState( const QString &key ) const
{
    QMap<QString, bool>::ConstIterator it = m_traceKeyDefaultState.find( key );
    return it == m_traceKeyDefaultState.end() || *it;
}

void FilterForm::enableTraceKeyByDefault( const QString &name, bool enabled )
{
    m_traceKeyDefaultState[name] = enabled;
}

