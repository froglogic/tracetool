#include "mainwindow.h"

#include "entryitemmodel.h"

#include <QtGui>

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_model(new EntryItemModel(this))
{
    setupUi(this);
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
