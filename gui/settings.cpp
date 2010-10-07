/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "settings.h"

#include "entryfilter.h"
#include "columnsinfo.h"
#include "../hooklib/tracelib_config.h"

#include <cassert>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStringList>

const char companyName[] = "froglogic";
const char productName[] = "appstalker";
const char sessionGroup[] = "Session";
const char databaseGroup[] = "Database";
const char serverGroup[] = "Server";
const char configGroup[] = "Configuration";

const int defaultSoftLimit = 1500000;
const int defaultHardLimit = defaultSoftLimit + 500000;

static QString defaultFile()
{
    return QDir::temp().absoluteFilePath("demo.trace");
}

Settings::Settings()
    : m_softLimit(-1),
      m_hardLimit(-1)
{
    m_entryFilter = new EntryFilter();
    registerRestorable("Filter", m_entryFilter);
    m_columnsInfo = new ColumnsInfo();
    registerRestorable("ColumnsInfo", m_columnsInfo);

    if (!load()) {
        qWarning() << "Failed to load settings";
    }
}

Settings::~Settings()
{
    delete m_entryFilter;
    delete m_columnsInfo;
}

bool Settings::save() const
{
    QSettings qs(QSettings::IniFormat, QSettings::UserScope,
                 companyName, productName);

    // [Database]
    qs.beginGroup(databaseGroup);
    qs.setValue("SoftLimit", m_softLimit);
    qs.setValue("HardLimit", m_hardLimit);
    qs.endGroup();

    // [Configuration]
    qs.beginGroup(configGroup);
    qs.setValue("Files", m_configFiles);
    qs.endGroup();

    // [Server]
    qs.beginGroup(serverGroup);
    qs.setValue("GUIPort", m_serverGUIPort);
    qs.setValue("TracePort", m_serverTracePort);
    qs.setValue("StartAutomatically", m_serverStartedAutomatically);
    qs.setValue("OutputFile", m_databaseFile);
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
    m_softLimit = qs.value("SoftLimit", defaultSoftLimit).toInt();
    m_hardLimit = qs.value("HardLimit", defaultHardLimit).toInt();
    qs.endGroup();

    // [Configuration]
    qs.beginGroup(configGroup);
    m_configFiles = qs.value("Files", QStringList()).toStringList();
    qs.endGroup();

    // [Server]
    qs.beginGroup(serverGroup);
    m_serverGUIPort = qs.value("GUIPort", TRACELIB_DEFAULT_PORT).toInt();
    m_serverTracePort = qs.value("TracePort", m_serverGUIPort + 1).toInt();
    m_serverStartedAutomatically = qs.value("StartAutomatically", true).toBool();
    m_databaseFile = qs.value("OutputFile", defaultFile()).toString();

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

// ### do some sanity checks on the two limit values
// ### E.g. minimum size possible, one bigger than the other
void Settings::setSoftLimit(int bytes)
{
    m_softLimit = bytes;
}

void Settings::setHardLimit(int bytes)
{
    // ### shrink database?
    m_hardLimit = bytes;
}

void Settings::addConfigurationFile(const QString &fileName)
{
    if (m_configFiles.contains(fileName))
        return;
    m_configFiles.prepend(fileName);
    while (m_configFiles.size() > 10)
        m_configFiles.removeLast();
}
