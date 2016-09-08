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
    Settings( const Settings &other ); // disabled
    void operator=( const Settings &rhs ); // disabled

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
