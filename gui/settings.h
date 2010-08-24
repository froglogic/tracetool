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

private:
    bool load();

    QMap<QString, RestorableObject*> m_restorables;
};

#endif
