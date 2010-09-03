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
    enum ColumnName { Time, Application, PID, Thread, File, Line, Function, Type,
#ifdef SHOW_VERBOSITY
                     Verbosity,
#endif
                     Message, StackPosition, LastColumnName };

    ColumnsInfo(QObject *parent = 0);

    bool isVisible( ColumnName cname ) const;
    void setVisible( ColumnName cname, bool visible );

    // from RestorableObject interface
    QVariant sessionState() const;
    bool restoreSessionState(const QVariant &state);

signals:
    void changed();

private:
    QVector<bool> m_visible;
};

#endif
