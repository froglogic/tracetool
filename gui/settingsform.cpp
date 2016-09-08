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

#include "settingsform.h"

#include "columnsinfo.h"

#include <QFontDialog>

#include <cassert>

const int RealIndexDataRole = Qt::UserRole;

SettingsForm::SettingsForm(Settings *settings,
                         QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_settings(settings)
{
    setupUi(this);

    connect(upButton, SIGNAL(clicked()),
            this, SLOT(moveUp()));
    connect(downButton, SIGNAL(clicked()),
            this, SLOT(moveDown()));
    connect(toInvisible, SIGNAL(clicked()),
            this, SLOT(moveToInvisible()));
    connect(toVisible, SIGNAL(clicked()),
            this, SLOT(moveToVisible()));
    connect(fontButton, SIGNAL(clicked()),
            this, SLOT(selectFont()));

    restoreSettings();
}

void SettingsForm::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsForm::moveToInvisible()
{
    int row = listWidgetVisible->currentRow();
    if (row != -1) {
        QListWidgetItem *ci = listWidgetVisible->takeItem(row);
        listWidgetInvisible->addItem(ci);
    }
}

void SettingsForm::moveToVisible()
{
    int row = listWidgetInvisible->currentRow();
    if (row != -1) {
        QListWidgetItem *ci = listWidgetInvisible->takeItem(row);
        listWidgetVisible->addItem(ci);
    }
}

void SettingsForm::moveUp()
{
    int row = listWidgetVisible->currentRow();
    if (row >= 1) {
        QListWidgetItem *item = listWidgetVisible->takeItem(row);
        listWidgetVisible->insertItem(row - 1, item);
        listWidgetVisible->setCurrentRow(row - 1);
    }
}

void SettingsForm::moveDown()
{
    int row = listWidgetVisible->currentRow();
    if (row + 1 < listWidgetVisible->count()) {
        QListWidgetItem *item = listWidgetVisible->takeItem(row);
        listWidgetVisible->insertItem(row + 1, item);
        listWidgetVisible->setCurrentRow(row + 1);
    }
}

void SettingsForm::saveSettings()
{
    QList<int> visibleColumns;
    for (int row = 0; row < listWidgetVisible->count(); ++row) {
        QListWidgetItem *item = listWidgetVisible->item(row);
        QVariant data = item->data(RealIndexDataRole);
        assert(data.isValid());
        bool ok;
        int realIndex = data.toInt(&ok);
        assert(ok);
        visibleColumns.append(realIndex);
    }
    QList<int> invisibleColumns;
    for (int row = 0; row < listWidgetInvisible->count(); ++row) {
        QListWidgetItem *item = listWidgetInvisible->item(row);
        QVariant data = item->data(RealIndexDataRole);
        assert(data.isValid());
        bool ok;
        int realIndex = data.toInt(&ok);
        assert(ok);
        invisibleColumns.append(realIndex);
    }

    m_settings->columnsInfo()->setSorting(visibleColumns, invisibleColumns);

    m_settings->setFont(m_currentFont);
}

void SettingsForm::restoreSettings()
{
    const ColumnsInfo *inf = m_settings->columnsInfo();

    const QList<int> visibleColumns = inf->visibleColumns();
    QListIterator<int> it(visibleColumns);
    while (it.hasNext()) {
        int realIndex = it.next();
        const QString name = inf->columnName(realIndex);
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(RealIndexDataRole, realIndex);
        listWidgetVisible->addItem(item);
    }

    const QList<int> invisibleColumns = inf->invisibleColumns();
    it = invisibleColumns;
    while (it.hasNext()) {
        int realIndex = it.next();
        const QString name = inf->columnName(realIndex);
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(RealIndexDataRole, realIndex);
        listWidgetInvisible->addItem(item);
    }

    m_currentFont = m_settings->font();
    fontButton->setText(m_currentFont.family());
    fontButton->setFont(m_currentFont);
}

void SettingsForm::selectFont()
{
    bool ok;
    QFont f = QFontDialog::getFont(&ok, m_currentFont, this, tr("Select Font"));
    if (ok) {
        m_currentFont = f;
        fontButton->setText(m_currentFont.family());
        fontButton->setFont(m_currentFont);
    }
}

