/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "mainwindow.h"

#include "configeditor.h"
#include "configuration.h"
#include "filterform.h"
#include "settingsform.h"
#include "entryitemmodel.h"
#include "watchtree.h"
#include "columnsinfo.h"
#include "storageview.h"
#include "applicationtable.h"
#include "fixedheaderview.h"
#include "entryfilter.h"

#include <cassert>
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtNetwork>

#include "../hooklib/tracelib.h"
#include "../server/database.h"
#include "../server/datagramtypes.h"

ServerSocket::ServerSocket(QObject *parent)
    : QTcpSocket(parent)
{
    connect(this, SIGNAL(readyRead()), SLOT(handleIncomingData()));
}

void ServerSocket::handleIncomingData()
{
    QByteArray data = readAll();
    QDataStream stream(&data, QIODevice::ReadOnly);

    quint32 magicCookie;
    stream >> magicCookie;
    if (magicCookie != MagicServerProtocolCookie) {
        disconnect();
        return;
    }

    quint32 protocolVersion;
    stream >> protocolVersion;
    assert(protocolVersion == 1);

    quint8 datagramType;
    stream >> datagramType;
    switch (static_cast<ServerDatagramType>(datagramType)) {
        case TraceFileNameDatagram: {
            QString traceFileName;
            stream >> traceFileName;
            emit traceFileNameReceived(traceFileName);
            break;
        }
        case TraceEntryDatagram: {
            TraceEntry te;
            stream >> te;
            emit traceEntryReceived(te);
            break;
        }
        case ProcessShutdownEventDatagram: {
            ProcessShutdownEvent ev;
            stream >> ev;
            emit processShutdown(ev);
            break;
        }
    }
}

MainWindow::MainWindow(Settings *settings,
                       QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_settings(settings),
      m_entryItemModel(NULL),
      m_watchTree(NULL),
      m_serverSocket(NULL),
      m_applicationTable(NULL),
      m_connectionStatusLabel(NULL),
      m_automaticServerProcess(NULL)
{
    setupUi(this);
    m_settings->registerRestorable("MainWindow", this);

    m_connectionStatusLabel = new QLabel(tr("Not connected"));
    statusBar()->addWidget(m_connectionStatusLabel);

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
    // replacing standard header for performance reasons
    FixedHeaderView *hv = new FixedHeaderView(9, // ### dynamic
                                              Qt::Vertical,
                                              tracePointsView);
    tracePointsView->setVerticalHeader(hv);

    // buttons
    connect(freezeButton, SIGNAL(clicked()),
            this, SLOT(toggleFreezeState()));

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
    connect(actionSettings, SIGNAL(triggered()),
	    this, SLOT(editSettings()));
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
    stopAutomaticServer();
}

void MainWindow::setDatabase(const QString &databaseFileName)
{
    QString errMsg;
    if (!setDatabase(databaseFileName, &errMsg)) {
        qWarning() << "Failed to load database " << databaseFileName << ": " << errMsg;
    }
}

bool MainWindow::setDatabase(const QString &databaseFileName, QString *errMsg)
{
    QString statusMsg;
    if (m_serverSocket) {
        if (m_settings->startServerAutomatically()) {
            statusMsg = tr("Connected to automatically started server, trace file: %1").arg(databaseFileName);
        } else {
            statusMsg = tr("Connected to server on port %1, trace file: %2").arg(m_settings->serverGUIPort()).arg(databaseFileName);
        }
    } else {
        statusMsg = tr("Not connected. Trace file: %1").arg(databaseFileName);
    }
    m_connectionStatusLabel->setText(statusMsg);

    // Delete model(s) that might have existed previously
    if (m_entryItemModel) {
	tracePointsView->setModel(NULL);
	delete m_entryItemModel; m_entryItemModel = NULL;
    }

    if (QFile::exists(databaseFileName)) {
        m_db = Database::open(databaseFileName, errMsg);
    } else {
        m_db = Database::create(databaseFileName, errMsg);
    }
    if (!m_db.isValid())
        return false;

    tracePointsSearchWidget->setTraceKeys(Database::seenGroupIds(m_db));
    m_filterForm->setTraceKeys(Database::seenGroupIds(m_db));

    m_entryItemModel = new EntryItemModel(m_settings->entryFilter(),
                                          m_settings->columnsInfo(), this);
    if (!m_entryItemModel->setDatabase(m_db, errMsg)) {
	delete m_entryItemModel; m_entryItemModel = NULL;
        return false;
    }

    if (!m_watchTree->setDatabase(m_db, errMsg)) {
	delete m_entryItemModel; m_entryItemModel = NULL;
        return false;
    }

    tracePointsSearchWidget->setTraceKeys(Database::seenGroupIds(m_db));
    m_filterForm->setTraceKeys(Database::seenGroupIds(m_db));

    m_applicationTable->setApplications(Database::tracedApplications(m_db));

    if (m_serverSocket) {
        connect(m_serverSocket, SIGNAL(traceEntryReceived(const TraceEntry &)),
                m_entryItemModel, SLOT(handleNewTraceEntry(const TraceEntry &)));
        connect(m_serverSocket, SIGNAL(traceEntryReceived(const TraceEntry &)),
                m_watchTree, SLOT(handleNewTraceEntry(const TraceEntry &)));
        connect(m_serverSocket, SIGNAL(traceEntryReceived(const TraceEntry &)),
                m_applicationTable, SLOT(handleNewTraceEntry(const TraceEntry &)));
        connect(m_serverSocket, SIGNAL(traceEntryReceived(const TraceEntry &)),
                this, SLOT(handleNewTraceEntry(const TraceEntry &)));
        connect(m_serverSocket, SIGNAL(processShutdown(const ProcessShutdownEvent &)),
                m_applicationTable, SLOT(handleProcessShutdown(const ProcessShutdownEvent &)));
    }
    connect( tracePointsSearchWidget, SIGNAL( searchCriteriaChanged( const QString &,
                                                                     const QStringList &,
                                                                     SearchWidget::MatchType ) ),
             m_entryItemModel, SLOT( highlightEntries( const QString &,
                                                       const QStringList &,
                                                       SearchWidget::MatchType ) ) );
    connect(tracePointsSearchWidget, SIGNAL(activeTraceKeyChanged(const QString &)),
            m_entryItemModel, SLOT(highlightTraceKey(const QString &)));

    connect( tracePointsClear, SIGNAL(clicked()),
             this, SLOT(clearTracePoints()));
    connect(m_settings->entryFilter(), SIGNAL(changed()),
            m_entryItemModel, SLOT(reApplyFilter()));
    connect(m_settings->columnsInfo(), SIGNAL(changed()),
            m_entryItemModel, SLOT(reApplyFilter()));

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
    dataSet.append(m_applicationTable->horizontalHeader()->saveState());
    return dataSet;
}

bool MainWindow::restoreSessionState(const QVariant &state)
{
    QList<QVariant> dataSet = state.value< QList<QVariant> >();
    if (dataSet.size() != 5)
        return false;
    QByteArray geo = dataSet[0].value<QByteArray>();
    QByteArray docks = dataSet[1].value<QByteArray>();
    QByteArray horizTableHeader = dataSet[2].value<QByteArray>();
    QByteArray watchTreeHeader = dataSet[3].value<QByteArray>();
    QByteArray applicationTableHeader = dataSet[4].value<QByteArray>();
    return restoreGeometry(geo)
        && restoreState(docks)
        && tracePointsView->horizontalHeader()->restoreState(horizTableHeader)
        && m_watchTree->header()->restoreState(watchTreeHeader)
        && m_applicationTable->horizontalHeader()->restoreState(applicationTableHeader);
}

void MainWindow::fileOpenTrace()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Open Trace"),
					      QDir::currentPath(),
					      tr("Trace Files (*.trace)"));
    if (fn.isEmpty())
	return;

    if (m_serverSocket) {
        disconnect(m_serverSocket, 0, 0, 0);
        serverSocketDisconnected();
    }
    stopAutomaticServer();

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

static QString locateTraceD()
{
    // XXX possibly retrieve executable and directory names
    // XXX from the build system
#ifdef Q_OS_WIN
    const QString exe = "traced.exe";
#else
    const QString exe = "traced";
#endif

    QDir ourDir = QCoreApplication::applicationDirPath();
    // binary package layout?
    if (ourDir.exists(exe))
        return ourDir.absoluteFilePath(exe);

    // try source build layout
    if (!ourDir.cdUp())
        return QString();
    if (!ourDir.cd("server"))
        return QString();
    if (ourDir.exists(exe))
        return ourDir.absoluteFilePath(exe);

    return QString();
}

bool MainWindow::startAutomaticServer()
{
    if (m_settings->databaseFile().isEmpty()) {
        showError(tr("Trace Daemon Error"),
                  tr("No log file specified. Cannot start daemon."));
        return false;
    }

    QString executable = locateTraceD();
    if (executable.isEmpty()) {
        m_connectionStatusLabel->setText(tr("Failed to locate "
                                            "'traced' "
                                            "executable."));
        return false;
    }
    m_automaticServerProcess = new QProcess(this);
    m_automaticServerProcess->
        setReadChannelMode(QProcess::MergedChannels);
    m_automaticServerProcess->
        setReadChannel(QProcess::StandardOutput);
    connect(m_automaticServerProcess,
            SIGNAL(readyRead()),
            this,
            SLOT(automaticServerOutput()));
    connect(m_automaticServerProcess,
            SIGNAL(error(QProcess::ProcessError)),
            this,
            SLOT(automaticServerError(QProcess::ProcessError)));
    connect(m_automaticServerProcess,
            SIGNAL(finished(int, QProcess::ExitStatus)),
            this,
            SLOT(automaticServerExit(int, QProcess::ExitStatus)));
    QStringList args;
    args << QString("--port=%1").arg(m_settings->serverTracePort());
    args << QString("--guiport=%1").arg(m_settings->serverGUIPort());
    args << m_settings->databaseFile();

    m_automaticServerProcess->start(executable, args);

    // QProcess::start() is async so we cannot guarantee
    // that the daemon has really started successfully
    // at this point.
    return true;
}

void MainWindow::stopAutomaticServer()
{
    if (!m_automaticServerProcess)
        return;

    // nobody is supposed to hear the death cry of this object.
    m_automaticServerProcess->disconnect();

    m_automaticServerProcess->close();
    delete m_automaticServerProcess;
    m_automaticServerProcess = 0;
}

void MainWindow::connectToServer()
{
    stopAutomaticServer();

    if (m_settings->startServerAutomatically()) {
        if (!startAutomaticServer()) {
            return;
        }
    }

    delete m_serverSocket;
    m_serverSocket = new ServerSocket(this);
    connect(m_serverSocket, SIGNAL(traceFileNameReceived(const QString &)),
            this, SLOT(setDatabase(const QString &)));
    connect(m_serverSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleConnectionError(QAbstractSocket::SocketError)));
    connect(m_serverSocket, SIGNAL(disconnected()),
            this, SLOT(serverSocketDisconnected()));
    m_serverSocket->connectToHost(QHostAddress::LocalHost, m_settings->serverGUIPort());
    m_connectionStatusLabel->setText(tr("Attempting to connect to server on port %1...").arg(m_settings->serverGUIPort()));
}

void MainWindow::handleConnectionError(QAbstractSocket::SocketError error)
{
    QString msg;
    switch (error) {
        case QAbstractSocket::ConnectionRefusedError:
            msg = tr("Failed to find storage server listening on port %1. Is traced running?").arg(m_settings->serverGUIPort());
            break;
        case QAbstractSocket::NetworkError:
            msg = tr("The network connection broke down.");
            break;
        default:
            break;
    }
    m_connectionStatusLabel->setText(tr("Not connected."));
    statusBar()->showMessage( msg, 2000 );
}

void MainWindow::serverSocketDisconnected()
{
    m_connectionStatusLabel->setText(tr("Not connected."));
    m_serverSocket->deleteLater();
    m_serverSocket = 0;
}

void MainWindow::automaticServerError(QProcess::ProcessError error)
{
    m_connectionStatusLabel->setText(tr("Traced daemon reported an "
                                        "error with code %1.")
                                     .arg(int(error)));
}

void MainWindow::automaticServerExit(int code,
                                     QProcess::ExitStatus status)
{
    if (status == QProcess::CrashExit) {
        showError(tr("Trace Daemon Error"),
                  tr("The trace daemon crashed"));
        return;
    }
    assert(status == QProcess::NormalExit);
    if (code == 0) {
        m_connectionStatusLabel->setText(tr("Traced daemon exited."));
    } else {
        m_connectionStatusLabel->setText(tr("Traced daemon exited "
                                            "with status code %1.")
                                         .arg(code));
    }
}

void MainWindow::automaticServerOutput()
{
    assert(m_automaticServerProcess != NULL);
    QByteArray raw = m_automaticServerProcess->readAll();
    QString output = QString::fromLocal8Bit(raw);

    showError(tr("Trace Daemon Output"),
              output);
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

void MainWindow::editSettings()
{
    SettingsForm form(m_settings, this);
    if (form.exec() == QDialog::Accepted) {
        updateColumns();
    }
}

void MainWindow::editStorage()
{
    QString oldFileName = m_settings->databaseFile();
    int oldServerGUIPort = m_settings->serverGUIPort();
    int oldServerTracePort = m_settings->serverTracePort();
    bool oldStartServerAutomatically = m_settings->startServerAutomatically();
    StorageView dlg(m_settings, this);
    if (dlg.exec() == QDialog::Accepted) {
        /* Determine if we need to reconnect (and/or restart!) the storage
         * server.
         * XXX Consider a way to enforce reconnecting (for instance, when
         * starting the server after the GUI).
         */
        bool needToReconnect = true;
        if (m_serverSocket &&
            m_settings->startServerAutomatically() == oldStartServerAutomatically &&
            m_settings->serverGUIPort() == oldServerGUIPort) {
            if (m_settings->startServerAutomatically()) {
                if (oldFileName == m_settings->databaseFile() &&
                    oldServerTracePort == m_settings->serverTracePort()) {
                    needToReconnect = false;
                }
            } else {
                needToReconnect = false;
            }
        }

        if (needToReconnect) {
            connectToServer();
        }
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
    Database::trimTo(m_db, 0);
    m_entryItemModel->reApplyFilter();
    m_watchTree->reApplyFilter();
    tracePointsSearchWidget->setTraceKeys( QStringList() );
    m_filterForm->setTraceKeys( QStringList() );
    m_applicationTable->setApplications( QList<TracedApplicationInfo>() );
}

void MainWindow::traceEntryDoubleClicked(const QModelIndex &index)
{
    const unsigned int id = m_entryItemModel->idForIndex(index);
    const QList<StackFrame> backtrace = Database::backtraceForEntry(m_db, id);
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

void MainWindow::addNewTraceKey( const QString &id )
{
    if ( !m_serverSocket || m_serverSocket->state() == QAbstractSocket::ConnectedState ) {
        Database::addGroupId( m_db, id );
        tracePointsSearchWidget->setTraceKeys( Database::seenGroupIds( m_db ) );
        m_filterForm->setTraceKeys( Database::seenGroupIds( m_db ) );
    }
}

void MainWindow::handleNewTraceEntry( const TraceEntry &e )
{
    tracePointsSearchWidget->addTraceKeys( Database::seenGroupIds( m_db ) );
    m_filterForm->addTraceKeys( Database::seenGroupIds( m_db ) );
}

