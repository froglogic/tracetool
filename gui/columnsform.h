/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef COLUMNSFORM_H
#define COLUMNSFORM_H

#include <QDialog>
#include "ui_columnsform.h"
#include "settings.h"

class ColumnsForm : public QDialog, private Ui::ColumnsForm
{
    Q_OBJECT
public:
    explicit ColumnsForm(Settings *settings,
                        QWidget *parent = 0, Qt::WindowFlags flags = 0);

protected:
    void accept();

private slots:
    void moveToInvisible();
    void moveToVisible();

private:
    void saveSettings();
    void restoreSettings();

    Settings* const m_settings;
};

#endif

