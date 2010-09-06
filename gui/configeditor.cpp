/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "configeditor.h"

#include "configuration.h"

#include <QMessageBox>

static QString modeToString(MatchingMode m)
{
    if (m == WildcardMatching)
        return "wildcard";
    else if (m == RegExpMatching)
        return "regexp";
    else
        return "strict";
}

ConfigEditor::ConfigEditor(Configuration *conf,
                           QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_conf(conf)
{
    setupUi(this);

    connect(processList, SIGNAL(currentRowChanged(int)),
            this, SLOT(currentProcessChanged(int)));

    connect(pushButton, SIGNAL(clicked()),
            this, SLOT(newConfig()));

    connect(pushButton_2, SIGNAL(clicked()),
            this, SLOT(deleteConfig()));

    connect(nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(processNameChanged(const QString&)));

    fillInConfiguration();
}

ConfigEditor::~ConfigEditor()
{
    delete m_conf;
}

void ConfigEditor::fillInConfiguration()
{
    for (int i = 0; i < m_conf->processCount(); ++i) {
        ProcessConfiguration *p = m_conf->process(i);
        processList->addItem(p->m_name);
    }

    if (processList->count() > 0)
        processList->setCurrentRow(0);
}

void ConfigEditor::currentProcessChanged(int row)
{
    if (row < 0)
        return;
    assert(row < m_conf->processCount());
    const ProcessConfiguration *p = m_conf->process(row);

    nameEdit->setText(p->m_name);

    // Output
    outputTypeEdit->setText(p->m_outputType);
    hostEdit->setText(p->m_outputOption["host"]);
    portEdit->setText(p->m_outputOption["port"]);

    // Serializer
    serializerTypeEdit->setText(p->m_serializerType);
    serializerOptionEdit->setText(p->m_serializerOption["beautifiedOutput"]);

    // Filters
    filterTable->clear();
    const QList<TracePointSets> &tpsets = p->m_tracePointSets;
    filterTable->setRowCount(tpsets.count());
    QListIterator<TracePointSets> it(tpsets);
    int filterRow = 0;
    while (it.hasNext()) {
        TracePointSets s = it.next();
        QString txt0, txt1, txt2;
        if (s.m_maxVerbosity >= 0) {
            txt0 = "maxVerbosity";
            txt1 = "";
            txt2 = QString::number(s.m_maxVerbosity);
        } else if (!s.m_pathFilter.isEmpty()) {
            txt0 = "pathfilter";
            txt1 = modeToString(s.m_pathFilterMode);
            txt2 = s.m_pathFilter;
        } else if (!s.m_functionFilter.isEmpty()) {
            txt0 = "functionfilter";
            txt1 = modeToString(s.m_functionFilterMode);
            txt2 = s.m_functionFilter;
        } else {
            txt0 = "unknown";
            txt1 = "unknown";
            txt2 = "unknown";
        }
        QTableWidgetItem *item0 = new QTableWidgetItem(txt0);
        QTableWidgetItem *item1 = new QTableWidgetItem(txt1);
        QTableWidgetItem *item2 = new QTableWidgetItem(txt2);
        filterTable->setItem(filterRow, 0, item0);
        filterTable->setItem(filterRow, 1, item1);
        filterTable->setItem(filterRow, 2, item2);
        ++filterRow;
    }
}

void ConfigEditor::newConfig()
{
    QString newProcess("application.exe");
    ProcessConfiguration *pc = new ProcessConfiguration;
    pc->m_name = newProcess;
    m_conf->addProcessConfiguration(pc);
    processList->addItem(newProcess);
    processList->setCurrentRow(processList->count() - 1);
}

void ConfigEditor::deleteConfig()
{
    if (QListWidgetItem *lwi = processList->currentItem()) {
        processList->takeItem( processList->row(lwi));
    }
}

void ConfigEditor::processNameChanged(const QString &text)
{
    if (QListWidgetItem *lwi = processList->currentItem()) {
        lwi->setText(text);
    }
}

void ConfigEditor::accept()
{
    save();
    QDialog::accept();
}

void ConfigEditor::save()
{
    QString errMsg;
    if (!m_conf->save(&errMsg)) {
        QMessageBox::critical(this, "Save Error", errMsg);
    }
}

