/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "filterform.h"

#include "../core/tracelib.h"

FilterForm::FilterForm(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
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
}

