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

#include "columnsinfo.h"

#include <cassert>
#include <cstdio>
#include <cmath>
#include <QStringList>

typedef QMap<QString, QVariant> StorageMap;

const char* const columnNames[] = {
    QT_TRANSLATE_NOOP("ColumnsInfo", "Time"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "Application"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "PID"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "Thread"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "File"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "Line"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "Function"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "Type"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "Key"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "Message"),
    QT_TRANSLATE_NOOP("ColumnsInfo", "Stack Position"),
};

const int numColumns = sizeof(columnNames) / sizeof(char*);

ColumnsInfo::ColumnsInfo(QObject *parent)
    : m_visualToReal(numColumns)
{
    setDefaultState();
}

int ColumnsInfo::columnCount() const
{
    return numColumns;
}

void ColumnsInfo::setDefaultState()
{
    // all columns visible, pre-defined order
    for (int i = 0; i < columnCount(); ++i) {
        m_visualToReal[i] = i;
    }
}

// never translated
QString ColumnsInfo::columnName(int realIndex) const
{
    assert(realIndex >= 0 && realIndex < columnCount());
    return columnNames[realIndex];
}

int ColumnsInfo::indexByName(const QString &name) const
{
    // ### introduce pre-built map if called often
    for (int i = 0; i < columnCount(); ++i) {
        if (columnName(i) == name)
            return i;
    }

    return -1;
}

// subject to translation
QString ColumnsInfo::columnCaption(int realIndex) const
{
    assert(realIndex >= 0 && realIndex < columnCount());
    return tr(columnNames[realIndex]);
}

bool ColumnsInfo::isVisible(int visualIndex) const
{
    assert(visualIndex >= 0 && visualIndex < columnCount());
    return m_visualToReal[visualIndex] >= 0;
}

QList<int> ColumnsInfo::visibleColumns() const
{
    QList<int> res;
    for (int i = 0; i < columnCount(); ++i) {
        int realIndex = m_visualToReal[i];
        if (realIndex >= 0)
            res.append(realIndex);
    }
    return res;
}

QList<int> ColumnsInfo::invisibleColumns() const
{
    QList<int> res;
    for (int i = 0; i < columnCount(); ++i) {
        int realIndex = m_visualToReal[i];
        if (realIndex < 0)
            res.append(-(realIndex + 1));
    }
    return res;
}

void ColumnsInfo::setSorting(const QList<int> &visible,
                             const QList<int> &invisible)
{
    assert(visible.count() + invisible.count() == columnCount());

    int pos = 0;
    QListIterator<int> it(visible);
    while (it.hasNext()) {
        int idx = it.next();
        assert(idx >= 0 && idx < columnCount());
        m_visualToReal[pos++] = idx;
    }

    it = invisible;
    while (it.hasNext()) {
        int idx = it.next();
        assert(idx >= 0 && idx < columnCount());
        m_visualToReal[pos++] = -idx - 1;
    }

    emit changed();
}

int ColumnsInfo::unmap(int visualIndex) const
{
    assert(visualIndex >= 0 && visualIndex < columnCount());
    int realIndex = m_visualToReal[visualIndex];
    assert(realIndex >= 0); // invisible column has no position
    return realIndex;
}

QVariant ColumnsInfo::sessionState() const
{
    QStringList namesVisible, namesInvisible;
    for (int i = 0; i < m_visualToReal.size(); ++i) {
        int realIndex = m_visualToReal[i];
        if (realIndex >= 0)
            namesVisible.append(columnName(realIndex));
        else
            namesInvisible.append(columnName(-(realIndex + 1)));
    }

    StorageMap map;
    map["VisibleColumns"] = namesVisible;
    map["InvisibleColumns"] = namesInvisible;
    return QVariant(map);
}

bool ColumnsInfo::restoreSessionState(const QVariant &state)
{
    // ### type checks
    const StorageMap map = state.value<StorageMap>();
    const QVariant visibleVariant = map["VisibleColumns"];
    const QVariant invisibleVariant = map["InvisibleColumns"];
    const QStringList namesVisible = visibleVariant.value<QStringList>();
    const QStringList namesInvisible = invisibleVariant.value<QStringList>();

    QList<int> visibleColumns, invisibleColumns;
    QVector<bool> restored(columnCount(), false);

    // visible columns
    QListIterator<QString> it(namesVisible);
    while (it.hasNext()) {
        QString name = it.next();
        int index = indexByName(name);
        if (index >= 0) {
            visibleColumns.append(index);
            restored[index] = true;
        } else {
#ifndef NDEBUG
            fprintf(stderr, "Unknown column name '%s'\n", qPrintable(name));
#endif
        }
    }

    // invisible columns
    it = namesInvisible;
    while (it.hasNext()) {
        QString name = it.next();
        int index = indexByName(name);
        if (index >= 0) {
            invisibleColumns.append(index);
            restored[index] = true;
        } else {
            fprintf(stderr, "Unknown column name '%s'\n", qPrintable(name));
        }
    }

    // unspecified columns (added in new version for example)
    // add them to the list of visible columns
    if (visibleColumns.count() + invisibleColumns.count() != columnCount()) {
        QVector<bool>::const_iterator it, end = restored.end();
        for (it = restored.begin(); it != end; ++it) {
            if (!*it) {
                visibleColumns.append(it - restored.begin());
            }
        }
    }

    setSorting(visibleColumns, invisibleColumns);

    return true;
}

