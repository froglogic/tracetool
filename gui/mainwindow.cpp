/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "config.h"
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
#ifdef Q_OS_WIN
#  include "jobobject.h"
#endif

#include <cassert>
#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtNetwork>

#include "../hooklib/tracelib.h"
#include "../server/database.h"
#include "../server/datagramtypes.h"

// duplicated in server/server.cpp
template <typename DatagramType, typename ValueType>
QByteArray serializeDatagram( DatagramType type, const ValueType *v )
{
    QByteArray payload;
    {
        static const quint32 ProtocolVersion = 1;

        QDataStream stream( &payload, QIODevice::WriteOnly );
        stream.setVersion( QDataStream::Qt_4_0 );
        stream << MagicServerProtocolCookie << ProtocolVersion << (quint8)type;
        if ( v ) {
            stream << *v;
        }
    }

    QByteArray data;
    {
        QDataStream stream( &data, QIODevice::WriteOnly );
        stream.setVersion( QDataStream::Qt_4_0 );
        stream << (quint16)payload.size();
        data.append( payload );
    }

    return data;
}

QByteArray serializeServerDatagram( ServerDatagramType type ) {
    return serializeDatagram( type, (int *)0 );
}

template <typename T>
QByteArray serializeServerDatagram( ServerDatagramType type, const T &v ) {
    return serializeDatagram( type, &v );
}

ServerSocket::ServerSocket(QObject *parent)
    : QTcpSocket(parent)
{
    connect(this, SIGNAL(readyRead()), SLOT(handleIncomingData()));
}

// Mostly duplicated in server/server.cpp (GUIConnection::handleIncomingData)
void ServerSocket::handleIncomingData()
{
    QDataStream stream(this);
    stream.setVersion(QDataStream::Qt_4_0);

    while (true) {
        static quint16 nextPayloadSize = 0;
        if (nextPayloadSize == 0) {
            if (bytesAvailable() < sizeof(nextPayloadSize)) {
                return;
            }
            stream >> nextPayloadSize;
        }

        if (bytesAvailable() < nextPayloadSize) {
            return;
        }

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
            case DatabaseNukeFinishedDatagram:
                emit databaseWasNuked();
                break;
        }
        nextPayloadSize = 0;
    }
}

MainWindow::MainWindow(Settings *settings,
#ifdef Q_OS_WIN
                       JobObject *job,
#endif
                       QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags),
      m_settings(settings),
      m_entryItemModel(NULL),
      m_watchTree(NULL),
      m_serverSocket(NULL),
      m_applicationTable(NULL),
      m_connectionStatusLabel(NULL),
      m_automaticServerProcess(NULL)
#ifdef Q_OS_WIN
      , m_job(job)
#endif
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
	delete m_entryItemModel; m_entryItemModel = NULL;
    }

    if (QFile::exists(databaseFileName)) {
        m_db = Database::open(databaseFileName, errMsg);
    } else {
        m_db = Database::create(databaseFileName, errMsg);
    }
    if (!m_db.isValid())
        return false;

    QStringList traceKeysNames = Database::seenGroupIds(m_db);
    tracePointsSearchWidget->setTraceKeys(traceKeysNames);
    m_filterForm->setTraceKeys(traceKeysNames);

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

    tracePointsSearchWidget->setTraceKeys(traceKeysNames);
    m_applicationTable->setApplications(Database::tracedApplications(m_db));

    if (m_serverSocket) {
        connect(m_serverSocket, SIGNAL(traceEntryReceived(const TraceEntry &)),
                this, SLOT(handleNewTraceEntry(const TraceEntry &)));
        connect(m_serverSocket, SIGNAL(processShutdown(const ProcessShutdownEvent &)),
                m_applicationTable, SLOT(handleProcessShutdown(const ProcessShutdownEvent &)));
        connect(m_serverSocket, SIGNAL(databaseWasNuked()),
                this, SLOT(databaseWasNuked()));
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
    m_entryItemModel->setCellFont(m_settings->font());

    return true;
}

static QByteArray saveHeaderViewSizes(QHeaderView *hv)
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);

    const int cnt = hv->count();
    stream << cnt;

    for (int i = 0; i < cnt; ++i) {
        stream << hv->sectionSize(i);
    }

    return result;
}

static bool restoreHeaderViewSizes(QHeaderView *hv, const QByteArray &data)
{
    QDataStream stream(data);

    int cnt = 0;
    stream >> cnt;
    if (stream.status() != QDataStream::Ok) {
        return false;
    }

    if (cnt != hv->count()) {
        return false;
    }

    for (int i = 0; i < cnt; ++i) {
        int size = 0;
        stream >> size;
        hv->resizeSection(i, size);
    }

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
    dataSet.append(saveHeaderViewSizes(tracePointsView->horizontalHeader()));
    return dataSet;
}

bool MainWindow::restoreSessionState(const QVariant &state)
{
    QList<QVariant> dataSet = state.value< QList<QVariant> >();
    if (dataSet.size() < 5)
        return false;
    QByteArray geo = dataSet[0].value<QByteArray>();
    QByteArray docks = dataSet[1].value<QByteArray>();
    QByteArray horizTableHeader = dataSet[2].value<QByteArray>();
    QByteArray watchTreeHeader = dataSet[3].value<QByteArray>();
    QByteArray applicationTableHeader = dataSet[4].value<QByteArray>();
    QByteArray horizTableHeaderSizes;
    if (dataSet.size() >= 6)
        horizTableHeaderSizes = dataSet[5].value<QByteArray>();

    bool success = restoreGeometry(geo);
    success &= restoreState(docks);

    /* Call reset() to work around strange QHeaderView behaviour in Qt 4.6 and
     * Qt 4.7.1; without this, calling count() after the restoreState() call
     * below will yield 0. Has something to do with the executePostedLayout()
     * call in QHeaderView::count().
     */
    tracePointsView->horizontalHeader()->reset();

    success &= tracePointsView->horizontalHeader()->restoreState(horizTableHeader);
    success &= m_watchTree->header()->restoreState(watchTreeHeader);
    success &= m_applicationTable->horizontalHeader()->restoreState(applicationTableHeader);
    if (dataSet.size() >= 6)
        success &= restoreHeaderViewSizes(tracePointsView->horizontalHeader(), horizTableHeaderSizes);
    return success;
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
    const QString txt = tr("<qt>tracelib Version %1\n"
                           "<br><br>"
                           "Copyright %2 froglogic GmbH\n"
                           "<br><br>"
                           "Built with Qt %3 on %4.")
        .arg(TRACELIB_VERSION_STR)
        .arg("2010, 2011")
        .arg(QT_VERSION_STR)
        .arg(__DATE__);
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

#ifdef Q_OS_WIN
    if (m_automaticServerProcess && m_job) {
        m_job->assignProcess(m_automaticServerProcess->pid()->hProcess);
    }
#endif

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
        m_entryItemModel->setCellFont(m_settings->font());
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
    if (QMessageBox::warning(this,
                             tr("%1 - About To Clear Trace Data")
                                .arg(windowTitle()),
                             tr("This will erase all trace data stored in the "
                                "currently viewed file\n"
                                "\n"
                                "%1\n"
                                "\n"
                                "This operation cannot be undone. Are you "
                                "sure you want to continue?" )
                                 .arg(m_settings->databaseFile()),
                               QMessageBox::Yes | QMessageBox::No,
                               QMessageBox::No) == QMessageBox::No) {
        return;
    }

    if ( m_serverSocket ) {
        tracePointsClear->setEnabled( false );
        m_serverSocket->write( serializeServerDatagram( DatabaseNukeDatagram ) );
    } else {
        Database::trimTo( m_db, 0 );
        databaseWasNuked();
    }
}

void MainWindow::databaseWasNuked()
{
    m_entryItemModel->clear();
    m_watchTree->reApplyFilter();
    tracePointsSearchWidget->setTraceKeys( QStringList() );
    m_filterForm->setTraceKeys( QStringList() );
    m_applicationTable->setApplications( QList<TracedApplicationInfo>() );
    tracePointsClear->setEnabled( true );
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
    QList<TraceKey>::ConstIterator it, end = e.traceKeys.end();
    for ( it = e.traceKeys.begin(); it != end; ++it ) {
        m_filterForm->enableTraceKeyByDefault( ( *it ).name, ( *it ).enabled );
    }

    // This trick used to update filtered keys check boxes state.
    // So, when the first trace entry received, we clean the list of existing
    // list of keys on the filter form and let them populate again with correct
    // check box states.
    static bool firstEntryPassed = false;

    if (!firstEntryPassed) {
        m_filterForm->setTraceKeys(QStringList());
        firstEntryPassed = true;
    }

    tracePointsSearchWidget->addTraceKeys( Database::seenGroupIds( m_db ) );
    m_filterForm->addTraceKeys( Database::seenGroupIds( m_db ) );

    // Function calls below were handled through connection to traceEntryReceived
    // signal, but moved here in order we can have much control on the execution
    // order. This will allow to synchronize the filter form and table model
    // updates.
    m_entryItemModel->handleNewTraceEntry(e);
    m_watchTree->handleNewTraceEntry(e);
    m_applicationTable->handleNewTraceEntry(e);
}

