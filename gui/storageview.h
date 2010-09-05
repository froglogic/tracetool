/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef STORAGEVIEW_H
#define STORAGEVIEW_H

#include <QDialog>
#include "ui_storageview.h"
#include "settings.h"

class StorageView : public QDialog, private Ui::StorageView
{
    Q_OBJECT
public:
    explicit StorageView(Settings *settings, QWidget *parent = 0);

    void accept();

    void restoreSettings();

private:
    void saveSettings();

    Settings* const m_settings;
};

#endif

