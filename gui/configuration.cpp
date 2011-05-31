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
    m_fileName = fileName;
    QFile f(m_fileName);
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

    f.close();

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

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "process")
            readProcessElement();
        else if (m_xml.name() == "tracekeys")
            readTraceKeysElement();
        else if (m_xml.name() == "storage")
            m_xml.skipCurrentElement(); // TODO: handle?
        else
            m_xml.raiseError(tr("Unexpected element <%1>").arg(m_xml.name().toString()));
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

void Configuration::readTraceKeysElement()
{
    assert(m_xml.isStartElement() && m_xml.name() == "tracekeys");

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "key")
            m_traceKeys.append(m_xml.readElementText());
        else
            m_xml.raiseError(tr("Unexpected element '%1' in tracekeys element")
                             .arg(m_xml.name().toString()));
    }
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

    TracePointSet tps;
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

void Configuration::readVerbosityFilter(TracePointSet *tps)
{
    assert(m_xml.isStartElement() && m_xml.name() == "verbosityfilter");

    Filter f;
    f.type = Filter::VerbosityFilter;
    f.term = m_xml.attributes().value("maxVerbosity").toString();
    tps->m_filters.append(f);
    m_xml.skipCurrentElement();
}

MatchingMode Configuration::parseMatchingMode(const QString &s)
{
    bool ok;
    MatchingMode mode = stringToMode(s, &ok);
    if (!ok)
        m_xml.raiseError(tr("Unknown matching mode %1").arg(s));

    return mode;
}

void Configuration::readPathFilter(TracePointSet *tps)
{
    assert(m_xml.isStartElement() && m_xml.name() == "pathfilter");

    Filter f;
    f.type = Filter::PathFilter;
    QString modeStr = m_xml.attributes().value("matchingmode").toString();
    f.matchingMode = parseMatchingMode(modeStr);
    f.term = m_xml.readElementText();
    tps->m_filters.append(f);
}

void Configuration::readFunctionFilter(TracePointSet *tps)
{
    assert(m_xml.isStartElement() && m_xml.name() == "functionfilter");

    Filter f;
    f.type = Filter::PathFilter;
    QString modeStr = m_xml.attributes().value("matchingmode").toString();
    f.matchingMode = parseMatchingMode(modeStr);
    f.term = m_xml.readElementText();
    tps->m_filters.append(f);
}

bool Configuration::save(QString *errMsg)
{
    assert(!m_fileName.isEmpty());
    QFile f(m_fileName);
    if (!f.open(QIODevice::WriteOnly)) {
        *errMsg = f.errorString();
        return false;
    }

    QXmlStreamWriter stream(&f);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("tracelibConfiguration");
    
    if (!m_traceKeys.isEmpty()) {
        stream.writeStartElement("tracekeys");
        QStringList::ConstIterator it, end = m_traceKeys.end();
        for (it = m_traceKeys.begin(); it != end; ++it) {
            stream.writeTextElement("key", *it);
        }
        stream.writeEndElement(); // tracekeys
    }

    QListIterator<ProcessConfiguration*> it(m_processes);
    while (it.hasNext()) {
        ProcessConfiguration *p = it.next();
        stream.writeStartElement("process");
        stream.writeTextElement("name", p->m_name);

        // <output>
        stream.writeStartElement("output");
        stream.writeAttribute("type",  p->m_outputType);
        QMapIterator<QString, QString> oit(p->m_outputOption);
        while (oit.hasNext()) {
            oit.next();
            stream.writeStartElement("option");
            stream.writeAttribute("name", oit.key());
            stream.writeCharacters(oit.value());
            stream.writeEndElement(); // option
        }
        stream.writeEndElement(); // output

        // <serializer>
        stream.writeStartElement("serializer");
        stream.writeAttribute("type",  p->m_serializerType);
        QMapIterator<QString, QString> sit(p->m_serializerOption);
        while (sit.hasNext()) {
            sit.next();
            stream.writeStartElement("option");
            stream.writeAttribute("name", sit.key());
            stream.writeCharacters(sit.value());
            stream.writeEndElement(); // option
        }
        stream.writeEndElement(); // serializer

        // <tracepointsets>
        QListIterator<TracePointSet> lit(p->m_tracePointSets);
        while (lit.hasNext()) {
            TracePointSet tps = lit.next();
            stream.writeStartElement("tracepointset");
            QString v = tps.m_variables ? "yes" : "no";
            stream.writeAttribute("variables", v);
//            stream.writeStartElement("matchallfilter");
            QList<Filter>::ConstIterator fit, fend = tps.m_filters.end();
            for ( fit = tps.m_filters.begin(); fit != fend; ++fit ) {
                switch ( fit->type ) {
                    case Filter::VerbosityFilter:
                        stream.writeStartElement("verbosityfilter");
                        stream.writeAttribute("maxVerbosity", fit->term);
                        stream.writeEndElement();
                        break;
                    case Filter::FunctionFilter:
                        stream.writeStartElement("functionfilter");
                        stream.writeAttribute("matchingmode", modeToString(fit->matchingMode));
                        stream.writeCharacters(fit->term);
                        stream.writeEndElement();
                        break;
                    case Filter::PathFilter:
                        stream.writeStartElement("pathfilter");
                        stream.writeAttribute("matchingmode", modeToString(fit->matchingMode));
                        stream.writeCharacters(fit->term);
                        stream.writeEndElement();
                        break;
                }
            }
 //         stream.writeEndElement(); // matchallfilter
            stream.writeEndElement(); // tracepointset
        }

        stream.writeEndElement(); // process
    }

    stream.writeEndElement(); // tracelibConfiguration
    stream.writeEndDocument();
    
    return true;
}

void Configuration::addProcessConfiguration(ProcessConfiguration *pc)
{
    m_processes.append(pc);
}

QString Configuration::modeToString(MatchingMode m)
{
    if (m == WildcardMatching)
        return "wildcard";
    else if (m == RegExpMatching)
        return "regexp";
    else
        return "strict";
}

MatchingMode Configuration::stringToMode(QString s, bool *bOk)
{
    if (bOk) *bOk = true;
    if (s == "wildcard")
        return WildcardMatching;
    if (s == "regexp")
        return RegExpMatching;
    else if (s == "strict")
        return StrictMatching;

    if (bOk) *bOk = false;
    return StrictMatching;
}

