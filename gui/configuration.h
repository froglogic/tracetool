/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cassert>
#include <QMap>
#include <QObject>
#include <QString>
#include <QXmlStreamReader>

enum MatchingMode { StrictMatching, WildcardMatching, RegExpMatching };

struct TracePointSets
{
    TracePointSets() : m_variables(true), m_maxVerbosity(-1) { }

    bool m_variables;
    int m_maxVerbosity;

    QString m_pathFilter;
    MatchingMode m_pathFilterMode;

    QString m_functionFilter;
    MatchingMode m_functionFilterMode;
};

struct ProcessConfiguration
{
    QString m_name;

    QString m_outputType;
    QMap<QString, QString> m_outputOption;

    QString m_serializerType;
    QMap<QString, QString> m_serializerOption;

    QList<TracePointSets> m_tracePointSets;
};

class Configuration : public QObject
{
    Q_OBJECT
public:
    Configuration();
    ~Configuration();

    bool load(const QString &fileName, QString *errMsg);

    int processCount() const { return m_processes.count(); }
    ProcessConfiguration* process(int num);

private:
    void readConfigurationElement();
    void readProcessElement();
    void readNameElement(ProcessConfiguration *proc);
    void readOutputElement(ProcessConfiguration *proc);
    void readOutputOption(ProcessConfiguration *proc);
    void readSerializerElement(ProcessConfiguration *proc);
    void readSerializerOption(ProcessConfiguration *proc);
    void readTracePointSetElement(ProcessConfiguration *proc);
    void readVerbosityFilter(TracePointSets *tps);
    void readPathFilter(TracePointSets *tps);
    void readFunctionFilter(TracePointSets *tps);

    MatchingMode parseMatchingMode(const QString &s);

    QXmlStreamReader m_xml;
    QList<ProcessConfiguration*> m_processes;
};

#endif

