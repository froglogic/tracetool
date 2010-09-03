/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QLineEdit>

class QPushButton;
class QStringList;

class UnlabelledLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    UnlabelledLineEdit( QWidget *parent );

    void setPlaceholderText( const QString &placeholderText );

protected:
    virtual void paintEvent( QPaintEvent *e );

private:
    QString m_placeholderText;
};

class SearchWidget : public QWidget
{
    Q_OBJECT
public:
    SearchWidget( const QStringList &fields,
                  QWidget *parent = 0 );

signals:
    void searchCriteriaChanged( const QString &term,
                                const QStringList &fields );

private slots:
    void termEdited( const QString &term );
    void fieldsChanged();

private:
    void emitSearchCriteria();

    UnlabelledLineEdit *m_lineEdit;
    QList<QPushButton *> m_fieldButtons;
};

#endif // !defined(SEARCHWIDGET_H)

