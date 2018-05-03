/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSqlDatabase>
#include <QTcpSocket>
#include <QStyledItemDelegate>
#include <QMessageBox>
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

class BacktraceMessageBox : public QMessageBox {
    Q_OBJECT
public:
    BacktraceMessageBox( QWidget *parent, const QString &title, const QString &text )
        : QMessageBox( QMessageBox::Information, title, text, QMessageBox::Ok, parent ) {
        setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
    virtual ~BacktraceMessageBox() {}
protected:
    virtual void resizeEvent(QResizeEvent *event) {
        setMinimumSize(minimumSizeHint());
        QWidget::resizeEvent( event );
    }
};

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

