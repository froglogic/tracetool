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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cassert>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QXmlStreamReader>

enum MatchingMode { StrictMatching, WildcardMatching, RegExpMatching };

struct Filter
{
    enum Type {
        PathFilter,
        FunctionFilter
    } type;
    QString term;
    MatchingMode matchingMode;
};

struct TracePointSet
{
    TracePointSet() : m_variables(true) { }

    bool m_variables;

    QList<Filter> m_filters;
};

struct ProcessConfiguration
{
    QString m_name;

    QString m_outputType;
    QMap<QString, QString> m_outputOption;

    QString m_serializerType;
    QMap<QString, QString> m_serializerOption;

    QList<TracePointSet> m_tracePointSets;
};

class Configuration : public QObject
{
    Q_OBJECT
public:
    Configuration();
    ~Configuration();

    /// Defines the collection of trace keys.
    typedef QMap<QString, bool> TraceKeys;

    /// Defines the const iterator for trace keys.
    typedef QMap<QString, bool>::ConstIterator TraceKeysConstIterator;

    /// Defines collection of storage settings.
    typedef QMap<QString, QString> StorageSettings;

    bool load(const QString &fileName, QString *errMsg);
    bool save(QString *errMsg);

    int processCount() const { return m_processes.count(); }
    ProcessConfiguration* process(int num);

    void setTraceKeys(const TraceKeys &traceKeys) { m_traceKeys = traceKeys; }
    const TraceKeys &traceKeys() const { return m_traceKeys; }

    void addProcessConfiguration(ProcessConfiguration *pc);

    static QString modeToString(MatchingMode m);
    static MatchingMode stringToMode(QString s, bool *bOk = 0);

private:
    void readConfigurationElement();
    void readProcessElement();
    void readTraceKeysElement();
    void readStorageElement();
    void readNameElement(ProcessConfiguration *proc);
    void readOutputElement(ProcessConfiguration *proc);
    void readOutputOption(ProcessConfiguration *proc);
    void readSerializerElement(ProcessConfiguration *proc);
    void readSerializerOption(ProcessConfiguration *proc);
    void readTracePointSetElement(ProcessConfiguration *proc);
    void readPathFilter(TracePointSet *tps);
    void readFunctionFilter(TracePointSet *tps);

    MatchingMode parseMatchingMode(const QString &s);

    QXmlStreamReader m_xml;
    QList<ProcessConfiguration*> m_processes;
    QString m_fileName;
    TraceKeys m_traceKeys;
    StorageSettings m_storageSettings;
};

#endif

