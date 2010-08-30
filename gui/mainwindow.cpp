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

    connect(action_Open_Trace, SIGNAL(triggered()),
	    this, SLOT(openTrace()));
    connect(actionQuit, SIGNAL(triggered()),
            qApp, SLOT(quit()));
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

void MainWindow::openTrace()
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

void MainWindow::showError(const QString &title,
			   const QString &message)
{
    QMessageBox::critical(this, title, message);
}
