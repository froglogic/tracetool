/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "mainwindow.h"

#include "entryitemmodel.h"

#include <QtGui>

MainWindow::MainWindow(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_settings(settings),
      m_model(NULL)
{
    setupUi(this);
    m_settings->registerRestorable("MainWindow", this);

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
    if (m_model) {
	tracePointsView->setModel(NULL);
	delete m_model; m_model = NULL;
    }

    m_model = new EntryItemModel(this);
    if (!m_model->setDatabase(databaseFileName,
			      m_settings->serverPort(),
			      errMsg)) {
	delete m_model; m_model = NULL;
        return false;
    }

    m_settings->setDatabaseFile(databaseFileName);

    tracePointsView->setModel(m_model);

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
