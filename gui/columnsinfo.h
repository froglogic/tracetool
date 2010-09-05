/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef ColumnsInfo_H
#define ColumnsInfo_H

#include "restorableobject.h"

#include <QObject>
#include <QString>
#include <QVector>

class ColumnsInfo : public QObject,
                    public RestorableObject
{
    Q_OBJECT
public:
    ColumnsInfo(QObject *parent = 0);

    int columnCount() const;

    QString columnName(int realIndex) const;
    int indexByName(const QString &name) const;
    QString columnCaption(int realIndex) const;

    bool isVisible(int visualIndex) const;
    QList<int> visibleColumns() const;
    QList<int> invisibleColumns() const;

    void setSorting(const QList<int> &visible,
                    const QList<int> &invisible);

    int unmap(int visibleIndex) const;

    // from RestorableObject interface
    QVariant sessionState() const;
    bool restoreSessionState(const QVariant &state);

signals:
    void changed();

private:
    void setDefaultState(); // does not emit changed() so far

    QVector<int> m_visualToReal;
};

#endif
