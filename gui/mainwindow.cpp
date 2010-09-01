/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "mainwindow.h"

#include "entryitemmodel.h"

#include <QtGui>
#include <QtSql>

#include "../core/tracelib.h"

MainWindow::MainWindow(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_settings(settings),
      m_model(NULL)
{
    setupUi(this);
    m_settings->registerRestorable("MainWindow", this);

    connect(freezeButton, SIGNAL(clicked()),
            this, SLOT(toggleFreezeState()));

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

bool MainWindow::rebuildWatchTree(const QString &databaseFileName, QString *errMsg)
{
    const QString driverName = "QSQLITE";
    if (!QSqlDatabase::isDriverAvailable(driverName)) {
        *errMsg = tr("Missing required %1 driver.").arg(driverName);
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(driverName, "watchmodel");
    db.setDatabaseName(databaseFileName);
    if (!db.open()) {
        *errMsg = db.lastError().text();
        return false;
    }

    QSqlQuery query( db );
    bool result = query.exec( "SELECT"
                "  process.name,"
                "  process.pid,"
                "  path_name.name,"
                "  trace_point.line,"
                "  function_name.name,"
                "  variable.name,"
                "  variable.type,"
                "  variable.value"
                " FROM"
                "  traced_thread,"
                "  process,"
                "  path_name,"
                "  trace_point,"
                "  function_name,"
                "  variable,"
                "  trace_entry"
                " WHERE"
                "  trace_entry.id IN ("
                "    SELECT"
                "      DISTINCT trace_entry_id"
                "    FROM"
                "      variable"
                "    WHERE"
                "      trace_entry_id IN ("
                "        SELECT"
                "          MAX(id)"
                "        FROM"
                "          trace_entry"
                "        GROUP BY trace_point_id"
                "      )"
                "  )"
                " AND"
                "  variable.trace_entry_id = trace_entry.id"
                " AND"
                "  traced_thread.id = trace_entry.traced_thread_id"
                " AND"
                "  process.id = traced_thread.process_id"
                " AND"
                "  trace_point.id = trace_entry.trace_point_id"
                " AND"
                "  path_name.id = trace_point.path_id"
                " AND"
                "  function_name.id = trace_point.function_id"
                " ORDER BY"
                "  process.name" );

    if (!result) {
        *errMsg = query.lastError().text();
        return false;
    }

    typedef QMap<QString, QTreeWidgetItem *> ItemMap;
    ItemMap applicationItems, sourceFileItems, functionItems;

    while ( query.next() ) {
        QTreeWidgetItem *applicationItem = 0;
        {
            const QString application = QString( "%1 (PID %2)" )
                                            .arg( query.value( 0 ).toString() )
                                            .arg( query.value( 1 ).toString() );
            ItemMap::ConstIterator it = applicationItems.find( application );
            if ( it != applicationItems.end() ) {
                applicationItem = *it;
            } else {
                applicationItem = new QTreeWidgetItem( watchPointTree,
                                                       QStringList() << application );
                applicationItems[ application ] = applicationItem;
            }
        }

        QTreeWidgetItem *sourceFileItem = 0;
        {
            const QString sourceFile = query.value( 2 ).toString();
            ItemMap::ConstIterator it = sourceFileItems.find( sourceFile );
            if ( it != sourceFileItems.end() ) {
                sourceFileItem = *it;
            } else {
                sourceFileItem = new QTreeWidgetItem( applicationItem,
                                                      QStringList() << sourceFile );
                sourceFileItems[ sourceFile ] = sourceFileItem;
            }
        }

        QTreeWidgetItem *functionItem = 0;
        {
            const QString function = QString( "%1 (line %2)" )
                                        .arg( query.value( 4 ).toString() )
                                        .arg( query.value( 3 ).toString() );
            ItemMap::ConstIterator it = functionItems.find( function );
            if ( it != functionItems.end() ) {
                functionItem = *it;
            } else {
                functionItem = new QTreeWidgetItem( sourceFileItem,
                                                    QStringList() << function );
                functionItems[ function ] = functionItem;
            }

        }

        using TRACELIB_NAMESPACE_IDENT(VariableType);

        const VariableType::Value varType = static_cast<VariableType::Value>( query.value( 6 ).toInt() );
        const QString varName = QString( "%1 (%2)" )
                                    .arg( query.value( 5 ).toString() )
                                    .arg( VariableType::valueAsString( varType ) );
        const QString varValue = query.value( 7 ).toString();
        new QTreeWidgetItem( functionItem,
                             QStringList() << QString() << varName << varValue );
    }

    return true;
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

    rebuildWatchTree(databaseFileName, errMsg);

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
        m_model->suspend();
    } else {
        m_model->resume();
        tracePointsView->verticalScrollBar()->setValue(tracePointsView->verticalScrollBar()->maximum());
    }
}

