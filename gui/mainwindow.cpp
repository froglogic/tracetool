/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "mainwindow.h"

#include "configeditor.h"
#include "configuration.h"
#include "filterform.h"
#include "columnsform.h"
#include "entryitemmodel.h"
#include "watchtree.h"
#include "columnsinfo.h"
#include "storageview.h"
#include "applicationtable.h"

#include <cassert>
#include <QtGui>
#include <QtSql>

#include "../hooklib/tracelib.h"
#include "../server/server.h"

MainWindow::MainWindow(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_settings(settings),
      m_entryItemModel(NULL),
      m_watchTree(NULL),
      m_server(NULL),
      m_applicationTable(NULL)
{
    setupUi(this);
    m_settings->registerRestorable("MainWindow", this);

    tracePointsSearchWidget->setFields( QStringList()
            << tr( "Application" )
            << tr( "File" )
            << tr( "Function" )
            << tr( "Message" ) );

    m_watchTree = new WatchTree(settings->entryFilter());
    tabWidget->addTab( m_watchTree, tr( "Watch Points" ) );

    m_applicationTable = new ApplicationTable;
    tabWidget->addTab(m_applicationTable, tr("Traced Applications"));

    connect(tracePointsView, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(traceEntryDoubleClicked(const QModelIndex &)));

    // buttons
    connect(freezeButton, SIGNAL(clicked()),
            this, SLOT(toggleFreezeState()));

    connect(toolBox, SIGNAL(currentChanged(int)),
           this, SLOT(toolBoxPageChanged(int)));

    // File menu
    connect(action_Open_Trace, SIGNAL(triggered()),
	    this, SLOT(fileOpenTrace()));
    connect(actionOpen_Configuration, SIGNAL(triggered()),
            this, SLOT(fileOpenConfiguration()));
    m_configFilesMenu = new QMenu(menu_File);
    connect(m_configFilesMenu, SIGNAL(aboutToShow()),
            this, SLOT(configFilesAboutToShow()));
    actionRecent_Configurations->setMenu(m_configFilesMenu);
    actionRecent_Configurations->setEnabled(!m_settings->hasConfigurationFiles());
    connect(actionQuit, SIGNAL(triggered()),
            qApp, SLOT(quit()));

    // Edit menu
    connect(actionColumns, SIGNAL(triggered()),
	    this, SLOT(editColumns()));
    connect(actionStorage, SIGNAL(triggered()),
	    this, SLOT(editStorage()));

    // Help menu
    connect(action_About, SIGNAL(triggered()),
            this, SLOT(helpAbout()));

    delete dummyWidget;
    m_filterForm = new FilterForm(m_settings);
    gridLayout->addWidget(m_filterForm);
    connect(m_filterForm, SIGNAL(filterApplied()),
            this, SLOT(filterChange()));
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
    m_filterForm->setTraceKeys( m_server->seenGroupIds() );

    m_entryItemModel = new EntryItemModel(m_settings->entryFilter(),
                                          m_settings->columnsInfo(), this);
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

    m_applicationTable->setApplications(m_server->tracedApplications());

    connect(m_server, SIGNAL(traceEntryReceived(const TraceEntry &)),
            m_entryItemModel, SLOT(handleNewTraceEntry(const TraceEntry &)));
    connect(m_server, SIGNAL(traceEntryReceived(const TraceEntry &)),
            m_watchTree, SLOT(handleNewTraceEntry(const TraceEntry &)));
    connect(m_server, SIGNAL(traceEntryReceived(const TraceEntry &)),
            m_applicationTable, SLOT(handleNewTraceEntry(const TraceEntry &)));
    connect(m_server, SIGNAL(processShutdown(const ProcessShutdownEvent &)),
            m_applicationTable, SLOT(handleProcessShutdown(const ProcessShutdownEvent &)));
    connect( tracePointsSearchWidget, SIGNAL( searchCriteriaChanged( const QString &,
                                                                     const QStringList &,
                                                                     SearchWidget::MatchType ) ),
             m_entryItemModel, SLOT( highlightEntries( const QString &,
                                                       const QStringList &,
                                                       SearchWidget::MatchType ) ) );
    connect( tracePointsClear, SIGNAL(clicked()),
             this, SLOT(clearTracePoints()));

    m_settings->setDatabaseFile(databaseFileName);

    tracePointsView->setModel(m_entryItemModel);

    return true;
}

QVariant MainWindow::sessionState() const
{
    QList<QVariant> dataSet;
    dataSet.append(saveGeometry()); // widget itself
    dataSet.append(saveState()); // dock window geometry and state
    dataSet.append(tracePointsView->horizontalHeader()->saveState());
    dataSet.append(m_watchTree->header()->saveState());
    return dataSet;
}

bool MainWindow::restoreSessionState(const QVariant &state)
{
    QList<QVariant> dataSet = state.value< QList<QVariant> >();
    if (dataSet.size() != 4)
        return false;
    QByteArray geo = dataSet[0].value<QByteArray>();
    QByteArray docks = dataSet[1].value<QByteArray>();
    QByteArray horizTableHeader = dataSet[2].value<QByteArray>();
    QByteArray watchTreeHeader = dataSet[3].value<QByteArray>();
    return restoreGeometry(geo)
        && restoreState(docks)
        && tracePointsView->horizontalHeader()->restoreState(horizTableHeader)
        && m_watchTree->header()->restoreState(watchTreeHeader);
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

void MainWindow::fileOpenConfiguration()
{
    QString fn = QFileDialog::getOpenFileName(this,
                                              tr("Open Configuration File"),
					      QDir::currentPath(),
					      tr("Configuration File (*.xml)"));
    if (fn.isEmpty())
	return;

    openConfigurationFile(fn);
}

bool MainWindow::openConfigurationFile(const QString &fileName)
{
    QString errMsg;
    Configuration *config = new Configuration();
    if (!config->load(fileName, &errMsg)) {
	showError(tr("Open Error"),
		  tr("Error opening configuration file: %1").arg(errMsg));
        delete config;
        return false;
    }

    ConfigEditor editor(config, this);
    editor.exec();

    m_settings->addConfigurationFile(fileName);
    actionRecent_Configurations->setEnabled(true);

    return true;
}

void MainWindow::configFilesAboutToShow()
{
    // refresh sub-menu with list of recently opened config files
    m_configFilesMenu->clear();
    const QStringList configFiles = m_settings->configurationFiles();
    QStringList::const_iterator it, end = configFiles.end();
    int index = 0;
    for (it = configFiles.begin(); it != end; ++it, ++index) {
        QString txt = tr("&%1. %2").arg(index).arg(*it);
        QAction *act = m_configFilesMenu->addAction(txt);
        act->setData(*it); // for retrieval in connected slot
        connect(act, SIGNAL(triggered()), this, SLOT(openRecentConfigFile()));
    }
}

void MainWindow::openRecentConfigFile()
{
    QAction *act = qobject_cast<QAction*>(sender());
    assert(act);
    assert(act->data().isValid());
    QString fileName = act->data().toString();
    assert(!fileName.isEmpty());
    (void)openConfigurationFile(fileName);
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

void MainWindow::postRestore()
{
    m_filterForm->restoreSettings();
    updateColumns();
}

void MainWindow::filterChange()
{
    // switch to entry view
    toolBox->setCurrentIndex(1);
}

void MainWindow::editColumns()
{
    ColumnsForm form(m_settings, this);
    if (form.exec() == QDialog::Accepted) {
        updateColumns();
    }
}

void MainWindow::editStorage()
{
    StorageView dlg(m_settings, this);
    if (dlg.exec() == QDialog::Accepted) {
        // ###?
    }
}

void MainWindow::updateColumns()
{
    const ColumnsInfo *ci = m_settings->columnsInfo();
    for (int i = 0; i < ci->columnCount(); ++i) {
        // ### could fetch names here, too
        tracePointsView->setColumnHidden(i, !ci->isVisible(i));
    }
}

void MainWindow::clearTracePoints()
{
    m_server->trimTo( 0 );
    m_entryItemModel->reApplyFilter();
    m_watchTree->reApplyFilter();
    m_filterForm->setTraceKeys( QStringList() );
    m_applicationTable->setApplications( QList<TracedApplicationInfo>() );
}

void MainWindow::traceEntryDoubleClicked(const QModelIndex &index)
{
    const unsigned int id = m_entryItemModel->idForIndex(index);
    const QList<StackFrame> backtrace = m_server->backtraceForEntry( id );
    if ( backtrace.isEmpty() ) {
        QMessageBox::information( this,
                                  tr( "Backtrace" ),
                                  tr( "This entry has no associated backtrace." ) );
        return;
    }

    QStringList lines;
    QList<StackFrame>::ConstIterator it, end = backtrace.end();
    size_t depth = 0;
    for ( it = backtrace.begin(); it != end; ++it ) {
        QString line = QString( "#%1 in %2: %3+0x%4" )
                            .arg( depth++ )
                            .arg( it->module )
                            .arg( it->function )
                            .arg( it->functionOffset, 0, 16 );
        if ( !it->sourceFile.isEmpty() ) {
            line += ": " + it->sourceFile + ":" + QString::number( it->lineNumber );
        }

        lines.append( line );
    }

    QMessageBox::information( this,
                              tr( "Backtrace" ),
                              QString( "<pre>%1</pre>" ).arg( lines.join( "\n" ) ) );
}


void MainWindow::toolBoxPageChanged( int index )
{
    if ( index == 0 && m_server ) {
        m_filterForm->setTraceKeys( m_server->seenGroupIds() );
    }
}

