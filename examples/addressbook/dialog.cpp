/*
    Copyright (c) 2009-10 froglogic GmbH. All rights reserved.

    This file is part of an example program for Squish---it may be used,
    distributed, and modified, without limitation.

    Note that the icons used are from KDE (www.kde.org) and subject to
    the KDE's license.
*/

#include "dialog.h"
#include "lineedit.h"
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    forenameLineEdit = new LineEdit;
    QLabel *forenameLabel = new QLabel(tr("&Forename:"));
    forenameLabel->setBuddy(forenameLineEdit);
    surnameLineEdit = new LineEdit;
    QLabel *surnameLabel = new QLabel(tr("&Surname:"));
    surnameLabel->setBuddy(surnameLineEdit);
    emailLineEdit = new LineEdit;
    QLabel *emailLabel = new QLabel(tr("&Email:"));
    emailLabel->setBuddy(emailLineEdit);
    phoneLineEdit = new LineEdit;
    QLabel *phoneLabel = new QLabel(tr("&Phone:"));
    phoneLabel->setBuddy(phoneLineEdit);
    okButton = new QPushButton(tr("&OK"));
    cancelButton = new QPushButton(tr("&Cancel"));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(forenameLabel, 0, 0);
    layout->addWidget(forenameLineEdit, 0, 1);
    layout->addWidget(surnameLabel, 1, 0);
    layout->addWidget(surnameLineEdit, 1, 1);
    layout->addWidget(emailLabel, 2, 0);
    layout->addWidget(emailLineEdit, 2, 1);
    layout->addWidget(phoneLabel, 3, 0);
    layout->addWidget(phoneLineEdit, 3, 1);
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout, 4, 0, 1, 2);
    setLayout(layout);

    connect(forenameLineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(updateUi()));
    connect(surnameLineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(updateUi()));
    connect(emailLineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(updateUi()));
    connect(phoneLineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(updateUi()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    updateUi();
    setWindowTitle(tr("Address Book - Add"));
}


void Dialog::updateUi()
{
    okButton->setEnabled(
            !(
                forenameLineEdit->text().isEmpty() ||
                surnameLineEdit->text().isEmpty() ||
                emailLineEdit->text().isEmpty() ||
                phoneLineEdit->text().isEmpty()
             ));
}
