/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "configeditor.h"

#include "configuration.h"

#include <QMessageBox>
#include <QComboBox>

ConfigEditor::ConfigEditor(Configuration *conf,
                           QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_conf(conf)
{
    setupUi(this);

    portEdit->setValidator(new QIntValidator(this));

    connect(processList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(currentProcessChanged(QListWidgetItem*, QListWidgetItem*)));

    connect(newConfigButton, SIGNAL(clicked()),
            this, SLOT(newConfig()));

    connect(delConfigButton, SIGNAL(clicked()),
            this, SLOT(deleteConfig()));

    connect(nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(processNameChanged(const QString&)));

    connect(addFilterButton, SIGNAL(clicked()),
            this, SLOT(addFilter()));

    connect(removeFilterButton, SIGNAL(clicked()),
            this, SLOT(removeFilter()));

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

void ConfigEditor::saveCurrentProcess(int row)
{
    if (row < 0)
        return;
    assert(row < m_conf->processCount());
    ProcessConfiguration *p = m_conf->process(row);

    p->m_name = nameEdit->text();

    // Output
    p->m_outputType = outputTypeEdit->text();
    p->m_outputOption["host"] = hostEdit->text();
    p->m_outputOption["port"] = portEdit->text();

    // Serializer
    p->m_serializerType = serializerTypeEdit->text();
    p->m_serializerOption["beautifiedOutput"] = serializerOptionEdit->text();

    // Filters
    const int rows = filterTable->rowCount();
    QList<TracePointSets> tpsets;
    for (int i = 0;i < rows;++i) {
        QTableWidgetItem *item1 = filterTable->item(i, 0);
        QTableWidgetItem *item3 = filterTable->item(i, 2);
        TracePointSets tps;
        if (item1->text() == "maxVerbosity") {
            tps.m_maxVerbosity = item3->text().toInt();
        } else if (item1->text() == "pathfilter") {
            if (QComboBox *combo = qobject_cast<QComboBox*>(filterTable->cellWidget(i, 1)))
                tps.m_pathFilterMode = Configuration::stringToMode(combo->currentText());
            tps.m_pathFilter = item3->text();
        } else if (item1->text() == "functionfilter") {
            if (QComboBox *combo = qobject_cast<QComboBox*>(filterTable->cellWidget(i, 1)))
                tps.m_functionFilterMode = Configuration::stringToMode(combo->currentText());
            tps.m_functionFilter = item3->text();
        }
        tpsets.append(tps);
    }
    p->m_tracePointSets = tpsets;
}

void ConfigEditor::currentProcessChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        return;
    int row = processList->row(current);
    assert(row < m_conf->processCount());
    if (previous) {
        int prevRow = processList->row(previous);
        saveCurrentProcess(prevRow);
    }
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
    filterTable->clearContents();
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
            txt1 = Configuration::modeToString(s.m_pathFilterMode);
            txt2 = s.m_pathFilter;
        } else if (!s.m_functionFilter.isEmpty()) {
            txt0 = "functionfilter";
            txt1 = Configuration::modeToString(s.m_functionFilterMode);
            txt2 = s.m_functionFilter;
        } else {
            txt0 = "unknown";
            txt1 = "unknown";
            txt2 = "unknown";
        }
        QTableWidgetItem *item0 = new QTableWidgetItem(txt0);
        QTableWidgetItem *item2 = new QTableWidgetItem(txt2);
        filterTable->setItem(filterRow, 0, item0);
        if (!txt1.isEmpty()) {
            QComboBox *combo = new QComboBox();
            combo->addItems(QStringList() << Configuration::modeToString(StrictMatching)
                                          << Configuration::modeToString(WildcardMatching)
                                          << Configuration::modeToString(RegExpMatching));
            combo->setCurrentIndex(combo->findText(txt1));
            filterTable->setCellWidget(filterRow, 1, combo);
        } else
            filterTable->setCellWidget(filterRow, 1, 0);
        filterTable->setItem(filterRow, 2, item2);
        ++filterRow;
    }
}

void ConfigEditor::newConfig()
{
    QString newProcess("application.exe");
    ProcessConfiguration *pc = new ProcessConfiguration;
    pc->m_name = newProcess;
    pc->m_outputType = "tcp";
    pc->m_outputOption["host"] = "127.0.0.1";
    pc->m_serializerType = "xml";
    pc->m_serializerOption["beautifiedOutput"] = "yes";
    m_conf->addProcessConfiguration(pc);
    processList->addItem(newProcess);
    processList->setCurrentRow(processList->count() - 1);
}

void ConfigEditor::deleteConfig()
{
    if (QListWidgetItem *lwi = processList->currentItem()) {
        processList->takeItem(processList->row(lwi));
    }
}

void ConfigEditor::addFilter()
{
    if (QListWidgetItem *lwi = processList->currentItem()) {
        const int row = processList->row(lwi);
        ProcessConfiguration *p = m_conf->process(row);
        TracePointSets s;
        s.m_functionFilterMode = StrictMatching;
        s.m_functionFilter = "functionName";
        QList<TracePointSets> &tpsets = p->m_tracePointSets;
        tpsets.append(s);
        currentProcessChanged(lwi, 0);
    }
}

void ConfigEditor::removeFilter()
{
    if (QListWidgetItem *lwi = processList->currentItem()) {
        const int row = processList->row(lwi);
        ProcessConfiguration *p = m_conf->process(row);
        p->m_tracePointSets.takeAt(filterTable->currentRow());
        currentProcessChanged(lwi, 0);
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
    saveCurrentProcess(processList->currentRow());
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

