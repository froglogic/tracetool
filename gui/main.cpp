#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QTableView>

#include "entryitemmodel.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    if (a.argc() != 2) {
        QMessageBox::critical(0, "Missing argument",
                              "Expected file name argument");
        return 1;
    }

    const QString databaseFileName = QFile::decodeName(a.argv()[1]);

    EntryItemModel m;
    QString errMsg;
    if (!m.setDatabase(databaseFileName, &errMsg)) {
        QMessageBox::critical(0, "Database error",
                              errMsg);
        return 2;
    }

    QTableView w;
    w.setModel(&m);
    w.show();

    return a.exec();
}
