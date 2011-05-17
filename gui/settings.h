/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include "restorableobject.h"

#include <QFont>
#include <QList>
#include <QStringList>

class EntryFilter;
class ColumnsInfo;

class Settings
{
public:
    Settings();
    ~Settings();

    bool save() const;

    void registerRestorable(const QString &name, RestorableObject *r);
    bool saveSession();
    bool restoreSession() const;

    // [Database]
    void setDatabaseFile(const QString &file) {
	m_databaseFile = file;
    }
    QString databaseFile() const { return m_databaseFile; }

    // [Server]
    void setServerGUIPort(int port) { m_serverGUIPort = port; }
    int serverGUIPort() const { return m_serverGUIPort; }

    void setServerTracePort(int port) { m_serverTracePort = port; }
    int serverTracePort() const { return m_serverTracePort; }

    bool startServerAutomatically() const { return m_serverStartedAutomatically; }
    void setStartServerAutomatically(bool b) { m_serverStartedAutomatically = b; }

    // [Filter]
    EntryFilter* entryFilter() { return m_entryFilter; }

    // [ColumnsInfo]
    ColumnsInfo* columnsInfo() { return m_columnsInfo; }

    // [Configuration]
    void addConfigurationFile(const QString &fileName);
    bool hasConfigurationFiles() const { return m_configFiles.isEmpty(); }
    QStringList configurationFiles() const { return m_configFiles; }

    // [Display]
    const QFont &font() const { return m_font; }
    void setFont( const QFont &font ) { m_font = font; }

private:
    bool load();

    QMap<QString, RestorableObject*> m_restorables;

    QString m_databaseFile;
    int m_serverGUIPort;
    int m_serverTracePort;
    EntryFilter* m_entryFilter;
    ColumnsInfo *m_columnsInfo;
    QStringList m_configFiles;
    bool m_serverStartedAutomatically;
    QFont m_font;
};

#endif
