/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSqlDatabase>
#include <QTcpSocket>
#include <QStyledItemDelegate>
#include "ui_mainwindow.h"
#include "settings.h"

class ApplicationTable;
class EntryItemModel;
class Server;
class WatchTree;
class FilterForm;
class QModelIndex;
struct TraceEntry;
struct ProcessShutdownEvent;
class QLabel;
class QProcess;
class JobObject;

class ServerSocket : public QTcpSocket
{
    Q_OBJECT
public:
    ServerSocket(QObject *parent = 0);

signals:
    void traceFileNameReceived(const QString &fn);
    void traceEntryReceived(const TraceEntry &entry);
    void processShutdown(const ProcessShutdownEvent &ev);
    void databaseWasNuked();

private slots:
    void handleIncomingData();
};

class CustomDateTimeFormattingDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    CustomDateTimeFormattingDelegate( QObject *obj ): QStyledItemDelegate( obj ) {}
    virtual QString	displayText ( const QVariant &value, const QLocale &locale ) const;
};

class MainWindow : public QMainWindow, private Ui::MainWindow,
                   public RestorableObject
{
    Q_OBJECT
public:
    explicit MainWindow(Settings *settings,
#ifdef Q_OS_WIN
                        JobObject *job,
#endif
                        QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MainWindow();

    void postRestore();
    void connectToServer();

    bool setDatabase(const QString &databaseFileName, QString *errMsg);

public slots:
    void setDatabase(const QString &databaseFileName);
    
protected:
    // from RestorableObject interface
    QVariant sessionState() const;
    bool restoreSessionState(const QVariant &state);

private slots:
    void fileOpenTrace();
    void fileOpenConfiguration();
    void configFilesAboutToShow();
    void openRecentConfigFile();
    void helpAbout();
    void toggleFreezeState();
    void editSettings();
    void editStorage();
    void updateColumns();
    void filterChange();
    void clearTracePoints();
    void traceEntryDoubleClicked(const QModelIndex &index);
#if 0
    void addNewTraceKey(const QString &id);
#endif
    void handleConnectionError(QAbstractSocket::SocketError error);
    void serverSocketDisconnected();
    void automaticServerError(QProcess::ProcessError error);
    void automaticServerExit(int code, QProcess::ExitStatus status);
    void automaticServerOutput();
    void handleNewTraceEntry(const TraceEntry &e);
    void databaseWasNuked();

private:
    bool openConfigurationFile(const QString &fileName);
    void showError(const QString &title, const QString &message);
    bool startAutomaticServer();
    void stopAutomaticServer();

    Settings* const m_settings;
    QSqlDatabase m_db;
    EntryItemModel* m_entryItemModel;
    WatchTree* m_watchTree;
    FilterForm *m_filterForm;
    ServerSocket *m_serverSocket;
    QMenu *m_configFilesMenu;
    ApplicationTable *m_applicationTable;
    QLabel *m_connectionStatusLabel;
    QProcess *m_automaticServerProcess;
#ifdef Q_OS_WIN
    JobObject *m_job;
#endif
};

#endif

