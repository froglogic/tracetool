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

#define setVisible(columnName)  \
    f->setVisible( ColumnsInfo::columnName, listWidgetVisible->findItems( #columnName, Qt::MatchExactly ).count() > 0 );

void ColumnsForm::saveSettings()
{
    ColumnsInfo *f = m_settings->columnsInfo();
    setVisible( Time );
    setVisible( Application );
    setVisible( PID );
    setVisible( Thread );
    setVisible( File );
    setVisible( Line );
    setVisible( Function );
    setVisible( Type );
    setVisible( Message );
    setVisible( StackPosition );
}

#define addColumnName(columnName)  \
    if ( f->isVisible( ColumnsInfo::columnName ) ) \
        listWidgetVisible->addItem( #columnName ); \
    else \
        listWidgetInvisible->addItem( #columnName ); 

void ColumnsForm::restoreSettings()
{
    ColumnsInfo *f = m_settings->columnsInfo();
    addColumnName( Time );
    addColumnName( Application );
    addColumnName( PID );
    addColumnName( Thread );
    addColumnName( File );
    addColumnName( Line );
    addColumnName( Function );
    addColumnName( Type );
    addColumnName( Message );
    addColumnName( StackPosition );
}
