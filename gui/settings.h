/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include "restorableobject.h"

#include <QList>

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
    void setServerPort(int port) { m_serverPort = port; }
    int serverPort() const { return m_serverPort; }
    
    void setSoftLimit(int bytes);
    int softLimit() const { return m_softLimit; }

    void setHardLimit(int bytes);
    int hardLimit() const { return m_hardLimit; }

    // [Filter]
    EntryFilter* entryFilter() { return m_entryFilter; }

    // [ColumnsInfo]
    ColumnsInfo* columnsInfo() { return m_columnsInfo; }

private:
    bool load();

    QMap<QString, RestorableObject*> m_restorables;

    QString m_databaseFile;
    int m_serverPort;
    EntryFilter* m_entryFilter;
    ColumnsInfo *m_columnsInfo;
    int m_softLimit, m_hardLimit;
};

#endif
