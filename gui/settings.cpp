#include "settings.h"

#include "../core/tracelib_config.h"

#include <cassert>
#include <QDebug>
#include <QSettings>

const char companyName[] = "froglogic";
const char productName[] = "appstalker";
const char sessionGroup[] = "Session";
const char databaseGroup[] = "Database";
const char serverGroup[] = "Server";

Settings::Settings()
{
    if (!load()) {
        qWarning() << "Failed to load settings";
    }
}

bool Settings::save() const
{
    QSettings qs(QSettings::IniFormat, QSettings::UserScope,
                 companyName, productName);

    // [Database]
    qs.beginGroup(databaseGroup);
    qs.setValue("File", m_databaseFile);
    qs.endGroup();

    // [Server]
    qs.beginGroup(serverGroup);
    qs.setValue("Port", m_serverPort);
    qs.endGroup();

    qs.sync();
    return qs.status() == QSettings::NoError;
}

bool Settings::load()
{
    QSettings qs(QSettings::IniFormat, QSettings::UserScope,
                 companyName, productName);

    // [Database]
    qs.beginGroup(databaseGroup);
    m_databaseFile = qs.value("File", QString()).toString();
    qs.endGroup();

    // [Server]
    qs.beginGroup(serverGroup);
    m_serverPort = qs.value("Port", TRACELIB_DEFAULT_PORT).toInt();
    qs.endGroup();

    return qs.status() == QSettings::NoError;

}

void Settings::registerRestorable(const QString &name, RestorableObject *r)
{
    assert(!m_restorables.contains(name));
    m_restorables.insert(name, r);
}

bool Settings::saveSession()
{
    QSettings qs(QSettings::IniFormat, QSettings::UserScope,
                 companyName, productName);

    qs.beginGroup(sessionGroup);
    QMapIterator<QString, RestorableObject*> it(m_restorables);
    while (it.hasNext()) {
        it.next();
        QString n = it.key();
        RestorableObject *r = it.value();
        qs.setValue(n, r->sessionState());
    }
    qs.endGroup();

    qs.sync();
    return qs.status() == QSettings::NoError;

}

bool Settings::restoreSession() const
{
    QSettings qs(QSettings::IniFormat, QSettings::UserScope,
                 companyName, productName);

    qs.beginGroup(sessionGroup);
    QMapIterator<QString, RestorableObject*> it(m_restorables);
    while (it.hasNext()) {
        it.next();
        QString n = it.key();
        RestorableObject *r = it.value();
        QVariant state = qs.value(n);
        if (state.isValid()) {
            if (!r->restoreSessionState(state)) {
                qWarning() << "Failed to restore session state for object"
                           << n;
            }
        }
    }

    qs.endGroup();

    return qs.status() == QSettings::NoError;
}

