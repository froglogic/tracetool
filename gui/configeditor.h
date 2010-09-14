/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef CONFIGEDITOR_H
#define CONFIGEDITOR_H

#include <QDialog>
#include "ui_configeditor.h"

class QListWidgetItem;
class QStackedWidget;

class Configuration;

class FilterTable;

struct TracePointSet;

class FilterTableItem : public QFrame
{
    Q_OBJECT
public:
    FilterTableItem(FilterTable *fTable, const TracePointSet &tpsets);

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
    void newConfig();
    void deleteConfig();
    void processNameChanged(const QString &text);
    void addFilter();
    void clearFilters();
    void serializerComboChanged(int index);
    void outputTypeComboChanged(int index);

private:
    void fillInConfiguration();
    void save();

    Configuration *m_conf;
    FilterTable *filterTable;
};

#endif

