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

#ifndef FILTERFORM_H
#define FILTERFORM_H

#include <QWidget>
#include "ui_filterform.h"
#include "settings.h"

class FilterForm : public QWidget, private Ui::FilterForm
{
    Q_OBJECT
public:
    explicit FilterForm(Settings *settings, QWidget *parent = 0);

    void setTraceKeys( const QStringList &keys );
    void addTraceKeys( const QStringList &keys );
    void enableTraceKeyByDefault( const QString &name, bool enabled );

signals:
    void filterApplied();

public slots:
    void apply();

    void restoreSettings();

private:
    void saveSettings();
    bool traceKeyDefaultState( const QString &key ) const;

    Settings* const m_settings;
    QMap<QString, bool> m_traceKeyDefaultState;
};

#endif

