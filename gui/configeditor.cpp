/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "configeditor.h"

#include "configuration.h"

#include <QMessageBox>
#include <QStackedWidget>
#include <QComboBox>

class FilterTable : public QWidget
{
public:
    FilterTable(QWidget *parent);

    void loadFilters(const QList<TracePointSets> &tpsets);
    void saveFilters(QList<TracePointSets> &tpsets);
    void addFilter();
    void removeFilter(FilterTableItem* fti);

    void clearContents();
};

FilterTable::FilterTable(QWidget *parent)
 : QWidget(parent)
{
    QVBoxLayout *vb = new QVBoxLayout;
    vb->setSpacing(0);
    vb->setAlignment(Qt::AlignTop);
    setLayout(vb);
}

void FilterTable::clearContents()
{
    QObjectList::const_iterator it = children().begin();
    while (it != children().end()) {
        if ((*it)->objectName() == "filter_table_item")
            delete *it;
        else
            ++it;
    }
}

void FilterTable::removeFilter(FilterTableItem* fti)
{
    delete fti;
}

void FilterTable::addFilter()
{
    TracePointSets s;
    s.m_functionFilterMode = StrictMatching;
    s.m_functionFilter = "functionName";
    static_cast<QBoxLayout*>(layout())->addWidget(new FilterTableItem(this, s), 0, Qt::AlignTop);
}

class VerbosityFilterHelper : public QWidget
{
public:
    VerbosityFilterHelper(const TracePointSets &tp);
    bool saveFilter(TracePointSets &tp);
private:
    QLineEdit *m_le;
};

VerbosityFilterHelper::VerbosityFilterHelper(const TracePointSets &tp)
{
    QHBoxLayout *layout = new QHBoxLayout;
    QComboBox *combo = new QComboBox();
    combo->addItems(QStringList() << "maxVerbosity");
    layout->addWidget(combo);
    m_le = new QLineEdit(QString::number(tp.m_maxVerbosity));
    m_le->setValidator(new QIntValidator(this));
    layout->addWidget(m_le);
    setLayout(layout);
}

bool VerbosityFilterHelper::saveFilter(TracePointSets &tp)
{
    tp.m_maxVerbosity = static_cast<QLineEdit*>(m_le)->text().toInt();
    return true;
}

class PathFilterHelper : public QWidget
{
public:
    PathFilterHelper(const TracePointSets &tp);
    bool saveFilter(TracePointSets &tp);
private:
    QLineEdit *m_le;
    QComboBox *m_combo;
};

PathFilterHelper::PathFilterHelper(const TracePointSets &tp)
{
    QHBoxLayout *layout = new QHBoxLayout;
    m_combo = new QComboBox();
    m_combo->addItems(QStringList() << Configuration::modeToString(StrictMatching)
                                    << Configuration::modeToString(WildcardMatching)
                                    << Configuration::modeToString(RegExpMatching));
    m_combo->setCurrentIndex(m_combo->findText(Configuration::modeToString(tp.m_pathFilterMode)));
    layout->addWidget(m_combo);
    m_le = new QLineEdit(tp.m_pathFilter);
    layout->addWidget(m_le);
    setLayout(layout);
}

bool PathFilterHelper::saveFilter(TracePointSets &tp)
{
    tp.m_pathFilter = m_le->text();
    tp.m_pathFilterMode = Configuration::stringToMode(m_combo->currentText());
    return !tp.m_pathFilter.isEmpty();
}

class FunctionFilterHelper : public QWidget
{
public:
    FunctionFilterHelper(const TracePointSets &tp);
    bool saveFilter(TracePointSets &tp);
private:
    QLineEdit *m_le;
    QComboBox *m_combo;
};

FunctionFilterHelper::FunctionFilterHelper(const TracePointSets &tp)
{
    QHBoxLayout *layout = new QHBoxLayout;
    m_combo = new QComboBox();
    m_combo->addItems(QStringList() << Configuration::modeToString(StrictMatching)
                                    << Configuration::modeToString(WildcardMatching)
                                    << Configuration::modeToString(RegExpMatching));
    m_combo->setCurrentIndex(m_combo->findText(Configuration::modeToString(tp.m_functionFilterMode)));
    layout->addWidget(m_combo);
    m_le = new QLineEdit(tp.m_functionFilter);
    layout->addWidget(m_le);
    setLayout(layout);
}

bool FunctionFilterHelper::saveFilter(TracePointSets &tp)
{
    tp.m_functionFilter = m_le->text();
    tp.m_functionFilterMode = Configuration::stringToMode(m_combo->currentText());
    return !tp.m_functionFilter.isEmpty();
}

FilterTableItem::FilterTableItem(FilterTable *fTable, const TracePointSets &tp)
: m_fTable(fTable)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setObjectName("filter_table_item");
    QHBoxLayout *hb = new QHBoxLayout;
    QComboBox *combo = new QComboBox();
    combo->addItems(QStringList() << "verbosity"
                                  << "path"
                                  << "function");
    connect(combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(filterComboChanged(int)));
    hb->addWidget(combo);
    m_sw = new QStackedWidget;
    m_sw->addWidget(new VerbosityFilterHelper(tp));
    m_sw->addWidget(new PathFilterHelper(tp));
    m_sw->addWidget(new FunctionFilterHelper(tp));

    if (tp.m_maxVerbosity >= 0) {
        m_sw->setCurrentIndex(0);
        combo->setCurrentIndex(0);
    } else if (!tp.m_pathFilter.isEmpty()) {
        m_sw->setCurrentIndex(1);
        combo->setCurrentIndex(1);
    } else if (!tp.m_functionFilter.isEmpty()) {
        m_sw->setCurrentIndex(2);
        combo->setCurrentIndex(2);
    }

    hb->addWidget(m_sw);

    QPushButton *pb = new QPushButton("x");
    connect(pb, SIGNAL(clicked()),
            this, SLOT(removeFilter()));
    hb->addWidget(pb);

    setLayout(hb);
}

void FilterTableItem::filterComboChanged(int idx)
{
    m_sw->setCurrentIndex(idx);
}

void FilterTableItem::removeFilter()
{
    m_fTable->removeFilter(this);
}

bool FilterTableItem::saveFilter(TracePointSets &tpsets)
{
    if (m_sw->currentIndex() == 0) {
        return static_cast<VerbosityFilterHelper*>(m_sw->currentWidget())->saveFilter(tpsets);
    } else if (m_sw->currentIndex() == 1) {
        return static_cast<PathFilterHelper*>(m_sw->currentWidget())->saveFilter(tpsets);
    } else if (m_sw->currentIndex() == 2) {
        return static_cast<FunctionFilterHelper*>(m_sw->currentWidget())->saveFilter(tpsets);
    }
    return false;
}

void FilterTable::loadFilters(const QList<TracePointSets> &tpsets)
{
    clearContents();
    QListIterator<TracePointSets> it(tpsets);
    while (it.hasNext()) {
        TracePointSets s = it.next();
        FilterTableItem *w = new FilterTableItem(this, s);
        static_cast<QBoxLayout*>(layout())->addWidget(w, 0, Qt::AlignTop);
    }
}

void FilterTable::saveFilters(QList<TracePointSets> &tpsets)
{
    QObjectList::const_iterator it = children().begin();
    while (it != children().end()) {
        if ((*it)->objectName() == "filter_table_item") {
            TracePointSets tpset;
            if (static_cast<FilterTableItem*>((*it))->saveFilter(tpset))
                tpsets.append(tpset);
        }
        ++it;
    }
}

ConfigEditor::ConfigEditor(Configuration *conf,
                           QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      m_conf(conf)
{
    setupUi(this);

    portEdit->setValidator(new QIntValidator(0, 65535, this));

    filterTable = new FilterTable(scrollArea);
    scrollArea->setWidget(filterTable);

    serializerComboBox->addItem("XML", "xml");
    serializerComboBox->addItem("Human Readable Text", "plaintext");
    connect(serializerComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(serializerComboChanged(int)));

    outputTypeComboBox->addItem("TCP/IP Connection", "tcp");
    outputTypeComboBox->addItem("Console", "stdout");
    connect(outputTypeComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(outputTypeComboChanged(int)));

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

    connect(clearFiltersButton, SIGNAL(clicked()),
            this, SLOT(clearFilters()));

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
    p->m_outputType = outputTypeComboBox->itemData(outputTypeComboBox->currentIndex(),
                                                   Qt::UserRole).toString();
    p->m_outputOption.clear();
    if (p->m_outputType == "tcp") {
        const QString hostValue = hostEdit->text();
        if (!hostValue.isEmpty())
            p->m_outputOption["host"] = hostValue;
        const QString portValue = portEdit->text();
        if (!portValue.isEmpty())
            p->m_outputOption["port"] = portValue;
    }

    // Serializer
    p->m_serializerType = serializerComboBox->itemData(serializerComboBox->currentIndex(),
                                                       Qt::UserRole).toString();
    p->m_serializerOption.clear();
    if (p->m_serializerType == "xml")
        p->m_serializerOption["beautifiedOutput"] = beautifiedCheckBox->isChecked() ? "yes" : "no";

    // Filters
    QList<TracePointSets> tpsets;
    filterTable->saveFilters(tpsets);
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
    const QString outputType = p->m_outputType;
    outputTypeComboBox->setCurrentIndex(outputTypeComboBox->findData(outputType, Qt::UserRole));
    if (outputType == "tcp") {
        hostEdit->setText(p->m_outputOption["host"]);
        portEdit->setText(p->m_outputOption["port"]);
    } else {
        hostEdit->setText(QString::null);
        portEdit->setText(QString::null);
    }

    // Serializer
    const QString serializerType = p->m_serializerType;
    serializerComboBox->setCurrentIndex(serializerComboBox->findData(serializerType, Qt::UserRole));
    if (serializerType == "xml")
        beautifiedCheckBox->setChecked(p->m_serializerOption["beautifiedOutput"] == "yes");
    else
        beautifiedCheckBox->setChecked(false);

    // Filters
    const QList<TracePointSets> &tpsets = p->m_tracePointSets;
    filterTable->loadFilters(tpsets);
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
        filterTable->addFilter();
    }
}

void ConfigEditor::clearFilters()
{
    if (QListWidgetItem *lwi = processList->currentItem()) {
        filterTable->clearContents();
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

void ConfigEditor::serializerComboChanged(int index)
{
    const bool xmlSerializer = serializerComboBox->itemData(index, Qt::UserRole).toString() == "xml";
    beautifiedLabel->setEnabled(xmlSerializer);
    beautifiedCheckBox->setEnabled(xmlSerializer);
}

void ConfigEditor::outputTypeComboChanged(int index)
{
    const bool tcp = outputTypeComboBox->itemData(index, Qt::UserRole).toString() == "tcp";
    hostEdit->setEnabled(tcp);
    portEdit->setEnabled(tcp);
    hostLabel->setEnabled(tcp);
    portLabel->setEnabled(tcp);
}

