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

#include "configeditor.h"

#include "configuration.h"

#include <QMessageBox>
#include <QStackedWidget>
#include <QComboBox>

class FilterTable : public QWidget
{
public:
    FilterTable(QWidget *parent);

    void loadFilters(const QList<TracePointSet> &tpsets);
    void saveFilters(QList<TracePointSet> *tpsets);
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
        if (qobject_cast<FilterTableItem*>(*it))
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
    Filter f;
    f.type = Filter::FunctionFilter;
    f.term = "functionName";
    f.matchingMode = StrictMatching;
    static_cast<QBoxLayout*>(layout())->addWidget(new FilterTableItem(this, f), 0, Qt::AlignTop);
}

class PathFilterHelper : public FilterHelper
{
public:
    PathFilterHelper(const Filter &f);
    bool saveFilter(TracePointSet *tp);
private:
    QLineEdit *m_le;
    QComboBox *m_combo;
};

PathFilterHelper::PathFilterHelper(const Filter &f)
{
    QHBoxLayout *layout = new QHBoxLayout;
    m_combo = new QComboBox();
    m_combo->addItems(QStringList() << Configuration::modeToString(StrictMatching)
                                    << Configuration::modeToString(WildcardMatching)
                                    << Configuration::modeToString(RegExpMatching));
    m_combo->setCurrentIndex(m_combo->findText(Configuration::modeToString(f.matchingMode)));
    layout->addWidget(m_combo);
    m_le = new QLineEdit(f.term);
    layout->addWidget(m_le);
    setLayout(layout);
}

bool PathFilterHelper::saveFilter(TracePointSet *tp)
{
    Filter f;
    f.type = Filter::PathFilter;
    f.term = m_le->text();
    f.matchingMode = Configuration::stringToMode(m_combo->currentText());
    tp->m_filters.append(f);
    return true;
}

class FunctionFilterHelper : public FilterHelper
{
public:
    FunctionFilterHelper(const Filter &f);
    bool saveFilter(TracePointSet *tp);
private:
    QLineEdit *m_le;
    QComboBox *m_combo;
};

FunctionFilterHelper::FunctionFilterHelper(const Filter &f)
{
    QHBoxLayout *layout = new QHBoxLayout;
    m_combo = new QComboBox();
    m_combo->addItems(QStringList() << Configuration::modeToString(StrictMatching)
                                    << Configuration::modeToString(WildcardMatching)
                                    << Configuration::modeToString(RegExpMatching));
    m_combo->setCurrentIndex(m_combo->findText(Configuration::modeToString(f.matchingMode)));
    layout->addWidget(m_combo);
    m_le = new QLineEdit(f.term);
    layout->addWidget(m_le);
    setLayout(layout);
}

bool FunctionFilterHelper::saveFilter(TracePointSet *tp)
{
    Filter f;
    f.type = Filter::FunctionFilter;
    f.term = m_le->text();
    f.matchingMode = Configuration::stringToMode(m_combo->currentText());
    tp->m_filters.append(f);
    return true;
}

FilterTableItem::FilterTableItem(FilterTable *fTable, const Filter &f)
: m_fTable(fTable)
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    QHBoxLayout *hb = new QHBoxLayout;
    QComboBox *combo = new QComboBox();
    combo->addItems(QStringList() << tr("Path")
                                  << tr("Function"));
    connect(combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(filterComboChanged(int)));
    hb->addWidget(combo);
    m_sw = new QStackedWidget;
    m_sw->addWidget(new PathFilterHelper(f));
    m_sw->addWidget(new FunctionFilterHelper(f));

    switch ( f.type ) {
        case Filter::PathFilter:
            m_sw->setCurrentIndex(1);
            combo->setCurrentIndex(1);
            break;
        case Filter::FunctionFilter:
            m_sw->setCurrentIndex(2);
            combo->setCurrentIndex(2);
            break;
    }

    hb->addWidget(m_sw);

    QPushButton *pb = new QPushButton("X");
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

bool FilterTableItem::saveFilter(TracePointSet *tpset)
{
    QWidget *current = m_sw->currentWidget();
    FilterHelper *helper = qobject_cast<FilterHelper*>(current);
    assert(helper);
    return helper->saveFilter(tpset);
}

void FilterTable::loadFilters(const QList<TracePointSet> &tpsets)
{
    clearContents();
    if ( tpsets.isEmpty() ) {
        return;
    }

    TracePointSet tpset = tpsets.first();
    QList<Filter>::ConstIterator it, end = tpset.m_filters.end();
    for ( it = tpset.m_filters.begin(); it != end; ++it ) {
        FilterTableItem *w = new FilterTableItem(this, *it);
        static_cast<QBoxLayout*>(layout())->addWidget(w, 0, Qt::AlignTop);
    }
}

void FilterTable::saveFilters(QList<TracePointSet> *tpsets)
{
    TracePointSet tpset;
    QObjectList::const_iterator it = children().begin();
    while (it != children().end()) {
        FilterTableItem *item = qobject_cast<FilterTableItem*>(*it);
        if (item) {
            item->saveFilter(&tpset);
        }
        ++it;
    }
    tpsets->append(tpset);
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

    connect(traceKeyList, SIGNAL(currentRowChanged(int)),
            this, SLOT(traceKeyItemActivated(int)));
    connect(addTraceKeyButton, SIGNAL(clicked()),
            this, SLOT(addTraceKey()));
    connect(removeTraceKeyButton, SIGNAL(clicked()),
            this, SLOT(removeTraceKey()));

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

    traceKeyList->clear();
    const Configuration::TraceKeys &keys = m_conf->traceKeys();
    Configuration::TraceKeysConstIterator it, end = keys.constEnd();
    for (it = keys.constBegin(); it != end; ++it)
        addTraceKey(it.key(), it.value(), false);

    updateTraceKeyButtons();
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
    QList<TracePointSet> tpsets;
    filterTable->saveFilters(&tpsets);
    p->m_tracePointSets = tpsets;
}

void ConfigEditor::saveTraceKeyList()
{
    QMap<QString, bool> traceKeys;
    for (int i = 0; i < traceKeyList->count(); ++i) {
        const QListWidgetItem *item = traceKeyList->item(i);
        traceKeys.insert(item->text(), bool(item->checkState() == Qt::Checked));
    }
    m_conf->setTraceKeys(traceKeys);
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
    const QList<TracePointSet> &tpsets = p->m_tracePointSets;
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
    if (processList->currentItem()) {
        filterTable->addFilter();
    }
}

void ConfigEditor::clearFilters()
{
    if (processList->currentItem()) {
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
    saveTraceKeyList();
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

void ConfigEditor::updateTraceKeyButtons()
{
    removeTraceKeyButton->setEnabled(traceKeyList->currentRow() != -1);
}

void ConfigEditor::traceKeyItemActivated(int)
{
    updateTraceKeyButtons();
}

void ConfigEditor::addTraceKey(const QString &key, bool enabled, bool edit)
{
    QListWidgetItem *newItem = new QListWidgetItem(key);
    newItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable |
                      Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    if (enabled)
        newItem->setCheckState(Qt::Checked);
    else
        newItem->setCheckState(Qt::Unchecked);

    traceKeyList->addItem(newItem);
    if (edit)
        traceKeyList->editItem(newItem);
}

void ConfigEditor::removeTraceKey()
{
    traceKeyList->takeItem(traceKeyList->currentRow());
}

