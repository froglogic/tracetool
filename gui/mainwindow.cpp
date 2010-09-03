/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "mainwindow.h"

#include "filterform.h"
#include "entryitemmodel.h"
#include "watchtree.h"

#include <QtGui>
#include <QtSql>

#include "../core/tracelib.h"
#include "../server/server.h"

MainWindow::MainWindow(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_settings(settings),
      m_entryItemModel(NULL),
      m_watchTree(NULL),
      m_server(NULL)
{
    setupUi(this);
    m_settings->registerRestorable("MainWindow", this);

    m_watchTree = new WatchTree(settings->entryFilter());
    tabWidget->addTab( m_watchTree, tr( "Watch Points" ) );

    // buttons
    connect(freezeButton, SIGNAL(clicked()),
            this, SLOT(toggleFreezeState()));
    connect(filterButton, SIGNAL(clicked()),
            this, SLOT(openFilterForm()));

    // File menu
    connect(action_Open_Trace, SIGNAL(triggered()),
	    this, SLOT(fileOpenTrace()));
    connect(actionQuit, SIGNAL(triggered()),
            qApp, SLOT(quit()));

    // Help menu
    connect(action_About, SIGNAL(triggered()),
            this, SLOT(helpAbout()));
}

MainWindow::~MainWindow()
{
}

bool MainWindow::setDatabase(const QString &databaseFileName, QString *errMsg)
{
    // Delete model(s) that might have existed previously
    if (m_entryItemModel) {
	tracePointsView->setModel(NULL);
	delete m_entryItemModel; m_entryItemModel = NULL;
    }

    delete m_server; m_server = NULL;
    // will create new db file if necessary
    m_server = new Server(databaseFileName, m_settings->serverPort(), this);

    m_entryItemModel = new EntryItemModel(m_settings->entryFilter(), this);
    if (!m_entryItemModel->setDatabase(databaseFileName, errMsg)) {
	delete m_entryItemModel; m_entryItemModel = NULL;
	delete m_server; m_server = NULL;
        return false;
    }

    if (!m_watchTree->setDatabase(databaseFileName, errMsg)) {
	delete m_entryItemModel; m_entryItemModel = NULL;
	delete m_server; m_server = NULL;
        return false;
    }

    connect(m_server, SIGNAL(traceEntryReceived(const TraceEntry &)),
            m_entryItemModel, SLOT(handleNewTraceEntry(const TraceEntry &)));
    connect(m_server, SIGNAL(traceEntryReceived(const TraceEntry &)),
            m_watchTree, SLOT(handleNewTraceEntry(const TraceEntry &)));

    m_settings->setDatabaseFile(databaseFileName);

    tracePointsView->setModel(m_entryItemModel);

    return true;
}

QVariant MainWindow::sessionState() const
{
    QList<QVariant> dataSet;
    dataSet.append(saveGeometry()); // widget itself
    dataSet.append(saveState()); // dock window geometry and state
    return dataSet;
}

bool MainWindow::restoreSessionState(const QVariant &state)
{
    QList<QVariant> dataSet = state.value< QList<QVariant> >();
    if (dataSet.size() != 2)
        return false;
    QByteArray geo = dataSet[0].value<QByteArray>();
    QByteArray docks = dataSet[1].value<QByteArray>();
    return restoreGeometry(geo) && restoreState(docks);
}

void MainWindow::fileOpenTrace()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Open Trace"),
					      QDir::currentPath(),
					      tr("Trace Files (*.trace)"));
    if (fn.isEmpty())
	return;

    QString errMsg;
    if (!setDatabase(fn, &errMsg)) {
	showError(tr("Open Error"),
		  tr("Error opening trace file: %1").arg(errMsg));
	return;
    }
}

void MainWindow::helpAbout()
{
    const QString title = tr("About %1").arg(windowTitle());
    const QString txt = tr("<qt>Copyright %1 froglogic GmbH\n"
                           "<br><br>"
                           "Built with Qt %2 on %3.")
        .arg(2010).arg(QT_VERSION_STR).arg(__DATE__);
    QMessageBox::information(this, title, txt);
}

void MainWindow::showError(const QString &title,
			   const QString &message)
{
    QMessageBox::critical(this, title, message);
}

void MainWindow::toggleFreezeState()
{
    if (freezeButton->isChecked()) {
        m_entryItemModel->suspend();
        m_watchTree->suspend();
    } else {
        m_entryItemModel->resume();
        m_watchTree->resume();
        tracePointsView->verticalScrollBar()->setValue(tracePointsView->verticalScrollBar()->maximum());
    }
}

void MainWindow::openFilterForm()
{
    FilterForm form(m_settings, this);
    if (form.exec() == QDialog::Accepted) {
        m_entryItemModel->reApplyFilter();
    }
}
