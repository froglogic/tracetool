/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef FILTERFORM_H
#define FILTERFORM_H

#include <QDialog>
#include "ui_filterform.h"
#include "settings.h"

class FilterForm : public QDialog, private Ui::FilterForm
{
    Q_OBJECT
public:
    explicit FilterForm(Settings *settings,
                        QWidget *parent = 0, Qt::WindowFlags flags = 0);

    Settings* const m_settings;
};

#endif

