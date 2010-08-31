#ifndef DIALOG_H
#define DIALOG_H
/*
    Copyright (c) 2009-10 froglogic GmbH. All rights reserved.

    This file is part of an example program for Squish---it may be used,
    distributed, and modified, without limitation.

    Note that the icons used are from KDE (www.kde.org) and subject to
    the KDE's license.
*/

#include <QtGui/QDialog>
#include <QtGui/QLineEdit>

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE


class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent=0);

    QString forename() const { return forenameLineEdit->text(); }
    QString surname() const { return surnameLineEdit->text(); }
    QString email() const { return emailLineEdit->text(); }
    QString phone() const { return phoneLineEdit->text(); }

private slots:
    void updateUi();

private:
    QLineEdit *forenameLineEdit;
    QLineEdit *surnameLineEdit;
    QLineEdit *emailLineEdit;
    QLineEdit *phoneLineEdit;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // DIALOG_H
