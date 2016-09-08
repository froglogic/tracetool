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
