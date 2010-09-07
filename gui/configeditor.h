/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef CONFIGEDITOR_H
#define CONFIGEDITOR_H

#include <QDialog>
#include "ui_configeditor.h"

class Configuration;

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
    void currentProcessChanged(int row);
    void saveCurrentProcess(int row);
    void newConfig();
    void deleteConfig();
    void processNameChanged(const QString &text);

private:
    void fillInConfiguration();
    void save();

    Configuration *m_conf;
};

#endif

