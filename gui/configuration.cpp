/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "configuration.h"

#include <QFile>

Configuration::Configuration()
{
}

Configuration::~Configuration()
{
    qDeleteAll(m_processes);
}

bool Configuration::load(const QString &fileName, QString *errMsg)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        *errMsg = f.errorString();
        return false;
    }

    m_xml.setDevice(&f);
    if (m_xml.readNextStartElement()) {
        if (m_xml.name() == "tracelibConfiguration") {
            readConfigurationElement();
        } else {
            m_xml.raiseError(tr("This is not a tracelib configuration file."));
        }
    }

    if (m_xml.hasError()) {
        *errMsg = m_xml.errorString();
        return false;
    }

    return true;
}

ProcessConfiguration* Configuration::process(int num)
{
    assert(num >= 0 && num < m_processes.count());
    return m_processes[num];
}

void Configuration::readConfigurationElement()
{
    assert(m_xml.isStartElement() && m_xml.name() == "tracelibConfiguration");

    if (m_xml.readNextStartElement()) {
        if (m_xml.name() == "process")
            readProcessElement();
        else
            m_xml.raiseError(tr("Missing process element."));
    }
}

void Configuration::readProcessElement()
{
    assert(m_xml.isStartElement() && m_xml.name() == "process");

    ProcessConfiguration *proc = new ProcessConfiguration();

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "name")
            readNameElement(proc);
        else if (m_xml.name() == "output")
            readOutputElement(proc);
        else if (m_xml.name() == "serializer")
            readSerializerElement(proc);
        else if (m_xml.name() == "tracepointset")
            readTracePointSetElement(proc);
        else
            m_xml.raiseError(tr("Unexpected element '%1' in process element")
                             .arg(m_xml.name().toString()));
    }

    if (m_xml.hasError())
        delete proc;
    else
        m_processes.append(proc);
}

void Configuration::readNameElement(ProcessConfiguration *proc)
{
    QString n = m_xml.readElementText();
    proc->m_name = n;
}

void Configuration::readOutputElement(ProcessConfiguration *proc)
{
    proc->m_outputType = m_xml.attributes().value("type").toString();

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "option")
            readOutputOption(proc);
        else
            m_xml.raiseError(tr("Unexpected element '%1' in output element")
                             .arg(m_xml.name().toString()));
    }            
}

void Configuration::readOutputOption(ProcessConfiguration *proc)
{
    QString n = m_xml.attributes().value("name").toString();
    QString val = m_xml.readElementText();
    proc->m_outputOption[n] = val;
}

void Configuration::readSerializerElement(ProcessConfiguration *proc)
{
    proc->m_serializerType = m_xml.attributes().value("type").toString();

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "option")
            readSerializerOption(proc);
        else
            m_xml.raiseError(tr("Unexpected element '%1' in serializer element")
                             .arg(m_xml.name().toString()));
    }            
}

void Configuration::readSerializerOption(ProcessConfiguration *proc)
{
    QString n = m_xml.attributes().value("name").toString();
    QString val = m_xml.readElementText();
    proc->m_serializerOption[n] = val;
}

void Configuration::readTracePointSetElement(ProcessConfiguration *proc)
{
    assert(m_xml.isStartElement() && m_xml.name() == "tracepointset");

    TracePointSets tps;
    tps.m_variables = m_xml.attributes().value("variables") == "yes";

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "verbosityfilter")
            readVerbosityFilter(&tps);
        else if (m_xml.name() == "pathfilter")
            readPathFilter(&tps);
        else if (m_xml.name() == "functionfilter")
            readFunctionFilter(&tps);
        else
            m_xml.raiseError(tr("Unexpected element '%1' in tracepointsets "
                                "element").arg(m_xml.name().toString()));
    }            

    if (!m_xml.hasError())
        proc->m_tracePointSets.append(tps);
}

void Configuration::readVerbosityFilter(TracePointSets *tps)
{
    assert(m_xml.isStartElement() && m_xml.name() == "verbosityfilter");

    tps->m_maxVerbosity =
        m_xml.attributes().value("maxVerbosity").toString().toInt();
    m_xml.skipCurrentElement();
}

MatchingMode Configuration::parseMatchingMode(const QString &s)
{
    if (s == "wildcard")
        return WildcardMatching;
    else if (s == "regexp")
        return RegExpMatching;
    else if (s == "strict")
        return StrictMatching;
    m_xml.raiseError(tr("Unknown matching mode %1").arg(s));
    
    return StrictMatching;
}

void Configuration::readPathFilter(TracePointSets *tps)
{
    assert(m_xml.isStartElement() && m_xml.name() == "pathfilter");

    QString modeStr = m_xml.attributes().value("matchingmode").toString();
    tps->m_pathFilterMode = parseMatchingMode(modeStr);

    QString p = m_xml.readElementText();
    tps->m_pathFilter = p;

}

void Configuration::readFunctionFilter(TracePointSets *tps)
{
    assert(m_xml.isStartElement() && m_xml.name() == "functionfilter");

    QString modeStr = m_xml.attributes().value("matchingmode").toString();
    tps->m_functionFilterMode = parseMatchingMode(modeStr);

    QString f = m_xml.readElementText();
    tps->m_functionFilter = f;
}
