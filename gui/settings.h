/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QList>
#include <QVariant>

class RestorableObject
{
public:
    virtual ~RestorableObject() { }

    virtual QVariant sessionState() const = 0;
    virtual bool restoreSessionState(const QVariant &state) = 0;
};

class Settings
{
public:
    Settings();

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

private:
    bool load();

    QMap<QString, RestorableObject*> m_restorables;

    QString m_databaseFile;
    int m_serverPort;
};

#endif
