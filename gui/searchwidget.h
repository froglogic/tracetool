/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QLineEdit>

class QLabel;
class QComboBox;
class QHBoxLayout;
class QPushButton;
class QRadioButton;
class QStringList;
class QVBoxLayout;

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
    enum MatchType {
        StrictMatch,
        WildcardMatch,
        RegExpMatch
    };

    SearchWidget( QWidget *parent = 0 );

    void setFields( const QStringList &fields );
    void setTraceKeys( const QStringList &keys );

signals:
    void searchCriteriaChanged( const QString &term,
                                const QStringList &fields,
                                SearchWidget::MatchType matchType );
    void activeTraceKeyChanged( const QString &activeKey,
                                const QStringList &inactiveKeys );

private slots:
    void termEdited( const QString &term );
    void traceKeyChanged( const QString &key );
    void emitSearchCriteria();

private:
    UnlabelledLineEdit *m_lineEdit;
    QList<QPushButton *> m_fieldButtons;
    QHBoxLayout *m_buttonLayout;
    QVBoxLayout *m_modifierLayout;
    QRadioButton *m_strictMatch;
    QRadioButton *m_wildcardMatch;
    QRadioButton *m_regexpMatch;
    QComboBox *m_activeTraceKeyCombo;
    QLabel *m_activeTraceKeyComboLabel;
};

#endif // !defined(SEARCHWIDGET_H)

