/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    void addTraceKeys( const QStringList &keys );

signals:
    void searchCriteriaChanged( const QString &term,
                                const QStringList &fields,
                                SearchWidget::MatchType matchType );
    void activeTraceKeyChanged( const QString &activeKey );

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

