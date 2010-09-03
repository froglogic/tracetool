/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "columnsform.h"

#include "columnsinfo.h"
#include "../core/tracelib.h"

ColumnsForm::ColumnsForm(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_settings(settings)
{
    setupUi(this);

    connect(toInvisible, SIGNAL(clicked()),
            this, SLOT(moveToInvisible()));
    connect(toVisible, SIGNAL(clicked()),
            this, SLOT(moveToVisible()));

    restoreSettings();
}

void ColumnsForm::accept()
{
    saveSettings();
    QDialog::accept();
}

void ColumnsForm::moveToInvisible()
{
    QListWidgetItem *ci = listWidgetVisible->currentItem();
    if ( ci ) {
        listWidgetVisible->takeItem( listWidgetVisible->currentRow() );
        listWidgetInvisible->addItem( ci->text() );
    } 
}

void ColumnsForm::moveToVisible()
{
    QListWidgetItem *ci = listWidgetInvisible->currentItem();
    if ( ci ) {
        listWidgetInvisible->takeItem( listWidgetInvisible->currentRow() );
        listWidgetVisible->addItem( ci->text() );
    } 
}

void ColumnsForm::saveSettings()
{
}

void ColumnsForm::restoreSettings()
{
    ColumnsInfo *f = m_settings->columnsInfo();
    if ( f->isVisible( ColumnsInfo::Time ) )
        listWidgetVisible->addItem( "Time" );
    else
        listWidgetInvisible->addItem( "Time" );
}
