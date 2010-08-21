#include <QApplication>
#include <QFile>
#include <QMessageBox>

#include "mainwindow.h"
#include "settings.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    if (a.argc() != 2) {
        QMessageBox::critical(0, "Missing argument",
                              "Expected file name argument");
        return 1;
    }

    const QString databaseFileName = QFile::decodeName(a.argv()[1]);

    Settings settings;
    QString errMsg;
    MainWindow mw(&settings);
    if (!mw.setDatabase(databaseFileName, &errMsg)) {
        QMessageBox::critical(0, "Database error",
                              errMsg);
        return 2;
    }

    settings.restoreSession();

    mw.show();

    int statusCode = a.exec();

    settings.saveSession();

    return statusCode;
}
