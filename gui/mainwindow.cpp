#include "mainwindow.h"

#include "entryitemmodel.h"

#include <QtGui>

MainWindow::MainWindow(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_settings(settings),
      m_model(new EntryItemModel(this))
{
    setupUi(this);
    m_settings->registerRestorable("MainWindow", this);
}

MainWindow::~MainWindow()
{
}

bool MainWindow::setDatabase(const QString &databaseFileName, QString *errMsg)
{
    if (!m_model->setDatabase(databaseFileName, errMsg)) {
        return false;
    }

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

