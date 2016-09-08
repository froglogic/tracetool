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

#ifndef CONFIGEDITOR_H
#define CONFIGEDITOR_H

#include <QDialog>
#include "ui_configeditor.h"

struct Filter;
class QListWidgetItem;
class QStackedWidget;

class Configuration;

class FilterTable;

struct TracePointSet;

class FilterTableItem : public QFrame
{
    Q_OBJECT
public:
    FilterTableItem(FilterTable *fTable, const Filter &f);

    bool saveFilter(TracePointSet *tpsets);

private slots:
    void filterComboChanged(int index);
    void removeFilter();

private:
    QStackedWidget *m_sw;
    FilterTable *m_fTable;
};

class FilterHelper : public QWidget
{
    Q_OBJECT
public:
    virtual bool saveFilter(TracePointSet *tp) = 0;
};

class ConfigEditor : public QDialog, private Ui::ConfigEditor
{
    Q_OBJECT
public:
    explicit ConfigEditor(Configuration *conf,
                          QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~ConfigEditor();

    bool load(const QString &fileName, QString *errMsg);

protected:
    void accept();

private slots:
    void currentProcessChanged(QListWidgetItem *, QListWidgetItem *);
    void saveCurrentProcess(int row);
    void saveTraceKeyList();
    void newConfig();
    void deleteConfig();
    void processNameChanged(const QString &text);
    void addFilter();
    void clearFilters();
    void serializerComboChanged(int index);
    void outputTypeComboChanged(int index);
    void traceKeyItemActivated(int row);

    /// Adds a new checkable item to the list widget of keys.
    /*!
        @param key      The new of the key. Default value is empty string.
        @param enabled  Sets the check box state. It is enabled by default.
        @param edit     Determines whether item should be in edit state after adding.
    */
    void addTraceKey(const QString &key = QString(), bool enabled = true, bool edit = true);
    void removeTraceKey();

private:
    void fillInConfiguration();
    void save();
    void updateTraceKeyButtons();

    Configuration *m_conf;
    FilterTable *filterTable;
};

#endif

