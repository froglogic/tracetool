/*
    Copyright (c) 2009-10 froglogic GmbH. All rights reserved.

    This file is part of an example program for Squish---it may be used,
    distributed, and modified, without limitation.

    Note that the icons used are from KDE (www.kde.org) and subject to
    the KDE's license.
*/

#include "dialog.h"
#include "lineedit.h"
#include "mainwindow.h"
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QMetaProperty>
#include <QtCore/QTimer>
#include <QtGui/QItemEditorFactory>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QCloseEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QKeySequence>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>
#include <QtGui/QTableWidget>
#include <QtGui/QToolBar>

#include <tracelib.h>

const int COLUMNS = 4;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dirty(false)
{
    QString path = ":/images/";
    QAction *fileNewAction = new QAction(QIcon(path + "filenew.png"),
                                         tr("&New"), this);
    fileNewAction->setShortcut(QKeySequence(tr("Ctrl+N")));
    QAction *fileOpenAction = new QAction(QIcon(path + "fileopen.png"),
                                          tr("&Open..."), this);
    fileOpenAction->setShortcut(QKeySequence(tr("Ctrl+O")));
    fileSaveAction = new QAction(QIcon(path + "filesave.png"),
                                          tr("&Save"), this);
    fileSaveAction->setShortcut(QKeySequence(tr("Ctrl+S")));
    fileSaveAsAction = new QAction(QIcon(path + "filesaveas.png"),
                                            tr("Save &As..."), this);
    QAction *fileQuitAction = new QAction(QIcon(path + "filequit.png"),
                                          tr("&Quit"), this);
    editAddAction = new QAction(QIcon(path + "editadd.png"),
                                tr("&Add..."), this);
    editAddAction->setShortcut(QKeySequence(tr("Ctrl+A")));
    editRemoveAction = new QAction(QIcon(path + "editdelete.png"),
                                   tr("&Remove..."), this);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(fileNewAction);
    fileMenu->addAction(fileOpenAction);
    fileMenu->addAction(fileSaveAction);
    fileMenu->addAction(fileSaveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(fileQuitAction);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(editAddAction);
    editMenu->addAction(editRemoveAction);

    QToolBar *fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(fileNewAction);
    fileToolBar->addAction(fileOpenAction);
    fileToolBar->addAction(fileSaveAction);

    QToolBar *editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(editAddAction);
    editToolBar->addAction(editRemoveAction);

    tableWidget = new QTableWidget;
    setCentralWidget(tableWidget);

    connect(fileNewAction, SIGNAL(triggered()), this, SLOT(fileNew()));
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));
    connect(fileSaveAction, SIGNAL(triggered()), this, SLOT(fileSave()));
    connect(fileSaveAsAction, SIGNAL(triggered()),
            this, SLOT(fileSaveAs()));
    connect(fileQuitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(editAddAction, SIGNAL(triggered()),
            this, SLOT(editAdd()));
    connect(editRemoveAction, SIGNAL(triggered()),
            this, SLOT(editRemove()));
    connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
            this, SLOT(setDirty()));
    connect(tableWidget,
            SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
            this, SLOT(updateUi()));

    QItemEditorFactory *editorFactory = new QItemEditorFactory;
    QItemEditorCreatorBase *stringEditorCreator = new
            QStandardItemEditorCreator<LineEdit>();
    editorFactory->registerEditor(QVariant::String, stringEditorCreator);
    QItemEditorFactory::setDefaultFactory(editorFactory);

    fileNew();
    statusBar()->showMessage("Ready", 5000);
    setWindowTitle(tr("Address Book"));
    updateUi();

    QTimer *t = new QTimer( this );
    connect( t, SIGNAL( timeout() ), SLOT( timerTriggered() ) );
    t->start( 500 );
}

void MainWindow::timerTriggered()
{
    static int row = 0;
    static int column = 0;
    TRACELIB_WATCH_MSG( "Qt eventloop still running",
                        TRACELIB_VAR(row) << TRACELIB_VAR(column) );
    if ( ++row == 10 ) {
        ++column;
        row = 0;
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!okToContinue())
        event->ignore();
    else
        event->accept();
}


void MainWindow::setDirty(bool on)
{
    dirty = on;
    updateUi();
}


void MainWindow::updateUi()
{
    bool editable = tableWidget->rowCount() > 0 ||
                    windowTitle().endsWith(tr("Unnamed"));
    editMenu->setEnabled(editable);
    editAddAction->setEnabled(editable);
    editRemoveAction->setEnabled(tableWidget->currentRow() > -1);
    fileSaveAction->setEnabled(dirty);
    fileSaveAsAction->setEnabled(tableWidget->rowCount() > 0);
}


bool MainWindow::okToContinue()
{
    if (!dirty)
        return true;
    int reply = QMessageBox::question(this, tr("Address Book"),
            tr("Save unsaved changes?"),
            QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
        return fileSave();
    if (reply == QMessageBox::Cancel)
        return false;
    return true;
}


void MainWindow::fileNew()
{
    if (!okToContinue())
        return;
    clear();
    setWindowTitle(tr("Address Book - Unnamed"));
    updateUi();
    TRACELIB_WATCH( TRACELIB_VAR(dirty) << TRACELIB_VAR(windowTitle()) << TRACELIB_VAR(filename) );
}


void MainWindow::clear()
{
    TRACELIB_TRACE;
    tableWidget->clear();
    tableWidget->setColumnCount(COLUMNS);
    tableWidget->setRowCount(0);
    tableWidget->setHorizontalHeaderLabels(QStringList()
            << tr("Forename") << tr("Surname") << tr("Email")
            << tr("Phone"));
    filename.clear();
    setDirty(false);
}


void MainWindow::fileOpen()
{
    if (!okToContinue())
        return;
    QString name = QFileDialog::getOpenFileName(this,
            tr("Address Book - Choose File"), ".",
            tr("Address Files (*.adr)"), 0,
            QFileDialog::DontUseNativeDialog);
    if (name.isEmpty())
        return;
    load(name);
}


void MainWindow::load(const QString &name)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        TRACELIB_ERROR_MSG( QString( "Failed to load input file '%1'" ).arg( name ).toUtf8().data() );
        QMessageBox::warning(this, tr("Address Book - Error"),
                tr("Failed to read file: %1").arg(file.errorString()));
        return;
    }
    clear();
    int errors = 0;
    QTextStream in(&file);
    in.setCodec("utf-8");
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(QChar('|'));
        if (parts.count() == COLUMNS) {
            int row = tableWidget->rowCount();
            tableWidget->setRowCount(row + 1);
            for (int column = 0; column < COLUMNS; ++column)
                tableWidget->setItem(row, column,
                        new QTableWidgetItem(parts[column]));
        }
        else
            errors += 1;
    }
#if QT_VERSION >= 0x040100
    tableWidget->resizeColumnsToContents();
#endif
    filename = name;
    setDirty(false);
    setWindowTitle(tr("Address Book - %1")
                   .arg(QFileInfo(filename).fileName()));
    if (errors)
        statusBar()->showMessage(tr("Loaded %1---but failed to read %2")
                .arg(filename).arg(errors), 30000);
    else
        statusBar()->showMessage(tr("Loaded %1").arg(filename), 5000);
}


bool MainWindow::fileSave()
{
    if (filename.isEmpty())
        return fileSaveAs();
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QMessageBox::warning(this, tr("Address Book - Error"),
                tr("Failed to write file: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("utf-8");
    for (int row = 0; row < tableWidget->rowCount(); ++row) {
        for (int column = 0; column < tableWidget->columnCount();
             ++column) {
            QString text = tableWidget->item(row, column)->text();
            text.replace("|", "");
            out << text;
            out << (column + 1 < tableWidget->columnCount() ? "|" : "\n");
        }
    }
    file.close();
    setDirty(false);
    return true;
}


bool MainWindow::fileSaveAs()
{
    QString name = QFileDialog::getSaveFileName(this,
            tr("Address Book - Save As"), ".",
            tr("Address Files (*.adr)"), 0,
            QFileDialog::DontUseNativeDialog);
    if (name.isEmpty())
        return false;
    if (!name.toLower().endsWith(".adr"))
        name += ".adr";
    filename = name;
    setWindowTitle(tr("Address Book - %1")
                   .arg(QFileInfo(filename).fileName()));
    return fileSave();
}


void MainWindow::editAdd()
{
    Dialog dialog(this);
    if (dialog.exec()) {
        TRACELIB_WATCH_MSG("Values returned by Add Entry dialog",
                           TRACELIB_VAR(dialog.forename())
                           << TRACELIB_VAR(dialog.surname())
                           << TRACELIB_VAR(dialog.email())
                           << TRACELIB_VAR(dialog.phone()));
        int row = tableWidget->rowCount() == 0
                ? 0 :tableWidget->currentRow();
        tableWidget->insertRow(row);
        tableWidget->setItem(row, 0,
                new QTableWidgetItem(dialog.forename()));
        tableWidget->setItem(row, 1,
                new QTableWidgetItem(dialog.surname()));
        tableWidget->setItem(row, 2,
                new QTableWidgetItem(dialog.email()));
        tableWidget->setItem(row, 3,
                new QTableWidgetItem(dialog.phone()));
        setDirty(true);
    }
}


void MainWindow::editRemove()
{
    int row = tableWidget->currentRow();
    if (row < 0 || row >= tableWidget->rowCount())
        return;

    if (QMessageBox::question(this, tr("Address Book - Delete"),
            tr("Delete '%1 %2'?")
            .arg(tableWidget->item(row, 0)->text())
            .arg(tableWidget->item(row, 1)->text()),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::No)
        return;
    tableWidget->removeRow(row);
    setDirty(true);
}
