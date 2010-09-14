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

struct Filter
{
    enum Type {
        VerbosityFilter,
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

    bool load(const QString &fileName, QString *errMsg);
    bool save(QString *errMsg);

    int processCount() const { return m_processes.count(); }
    ProcessConfiguration* process(int num);

    void addProcessConfiguration(ProcessConfiguration *pc);

    static QString modeToString(MatchingMode m);
    static MatchingMode stringToMode(QString s, bool *bOk = 0);

private:
    void readConfigurationElement();
    void readProcessElement();
    void readNameElement(ProcessConfiguration *proc);
    void readOutputElement(ProcessConfiguration *proc);
    void readOutputOption(ProcessConfiguration *proc);
    void readSerializerElement(ProcessConfiguration *proc);
    void readSerializerOption(ProcessConfiguration *proc);
    void readTracePointSetElement(ProcessConfiguration *proc);
    void readVerbosityFilter(TracePointSet *tps);
    void readPathFilter(TracePointSet *tps);
    void readFunctionFilter(TracePointSet *tps);

    MatchingMode parseMatchingMode(const QString &s);

    QXmlStreamReader m_xml;
    QList<ProcessConfiguration*> m_processes;
    QString m_fileName;
};

#endif

