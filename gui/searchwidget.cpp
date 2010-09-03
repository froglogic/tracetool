/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "searchwidget.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QStringList>
#include <QStyle>
#include <QStyleOptionFrameV2>
#include <QVBoxLayout>

UnlabelledLineEdit::UnlabelledLineEdit( QWidget *parent )
{
}

void UnlabelledLineEdit::setPlaceholderText( const QString &placeholderText )
{
    m_placeholderText = placeholderText;
    update();
}

void UnlabelledLineEdit::paintEvent( QPaintEvent *e )
{
    QLineEdit::paintEvent( e );
    if ( !hasFocus() && text().isEmpty() ) {
        QPainter p( this );
        QFont f = font();
        f.setStyle( QFont::StyleItalic );
        p.setFont( f );
        p.setPen( QPen( Qt::gray ) );

        QStyleOptionFrameV2 opt;
        initStyleOption( &opt );

        QRect r = style()->subElementRect( QStyle::SE_LineEditContents, &opt, this );

        p.drawText( r, Qt::AlignLeft | Qt::AlignVCenter, m_placeholderText );
    }
}

SearchWidget::SearchWidget( const QStringList &fields, QWidget *parent )
    : QWidget( parent ),
    m_lineEdit( 0 )
{
    m_lineEdit = new UnlabelledLineEdit( this );
    connect( m_lineEdit, SIGNAL( textEdited( const QString & ) ),
             this, SLOT( termEdited( const QString & ) ) );
    m_lineEdit->setPlaceholderText( "Search trace data..." );

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QStringList::ConstIterator it, end = fields.end();
    for ( it = fields.begin(); it != end; ++it ) {
        QPushButton *fieldButton = new QPushButton( *it );
        connect( fieldButton, SIGNAL( clicked() ),
                 this, SLOT( fieldsChanged() ) );

        fieldButton->setCheckable( true );

        QFont f = fieldButton->font();
        f.setPointSize( ( f.pointSize() * 90 ) / 100 );
        fieldButton->setFont( f );

        buttonLayout->addWidget( fieldButton );

        m_fieldButtons.append( fieldButton );

        fieldButton->hide();

    }
    buttonLayout->addStretch();

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( m_lineEdit );
    layout->addLayout( buttonLayout );
    layout->addStretch();
}

void SearchWidget::emitSearchCriteria()
{
    QStringList selectedFields;
    QList<QPushButton *>::ConstIterator it, end = m_fieldButtons.end();
    for ( it = m_fieldButtons.begin(); it != end; ++it ) {
        if ( ( *it )->isChecked() ) {
            selectedFields.append( ( *it )->text() );
        }
    }
    emit searchCriteriaChanged( m_lineEdit->text(), selectedFields );
}

void SearchWidget::termEdited( const QString &newTerm )
{
    QList<QPushButton *>::ConstIterator it, end = m_fieldButtons.end();
    for ( it = m_fieldButtons.begin(); it != end; ++it ) {
        ( *it )->setVisible( !newTerm.isEmpty() );
    }
    emitSearchCriteria();
}

void SearchWidget::fieldsChanged()
{
    emitSearchCriteria();
}

