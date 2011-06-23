/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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

