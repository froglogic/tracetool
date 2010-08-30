#include <QApplication>
#include <QFile>
#include <QMessageBox>

#include "mainwindow.h"
#include "settings.h"
#include "../convertdb/getopt.h"

#include <iostream>
#include <string>

namespace Error
{
    const int None = 0;
    const int CommandLineArgs = 1;
    const int Open = 2;
};

using namespace std;

static void printUsage(const string &app)
{
    cout << "Usage: " << app << " --help" << endl
         << "       " << app << " [--port <port>] <.trace-file>" << endl;
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    GetOpt opt;
    bool help;
    QString traceFile, portStr;
    opt.addSwitch("help", &help);
    opt.addOption('p', "port", &portStr);
    opt.addOptionalArgument("trace_file", &traceFile);
    if (!opt.parse()) {
        cout << "Invalid command line argument. Try --help." << endl;
        return Error::CommandLineArgs;
    }

    if (help) {
        printUsage(argv[0]);
        return Error::None;
    }
    if (!traceFile.isEmpty() &&
        !traceFile.endsWith(".trace", Qt::CaseInsensitive)) {
        // ### Too picky? No real technical reason so far. But
        // ### helps the IDE and who knows what else in the
        // ### future. Click a file type association that allows
        // ### opening traces by double-click.
        cout << "Trace file is expected to have a "
             << ".trace suffix. " << endl;
        return Error::CommandLineArgs;
    }
    int portOverride = -1;
    if (!portStr.isEmpty()) {
        bool ok;
        portOverride = portStr.toInt(&ok);
        if (!ok) {
            cout << "Invalid port number '"
                 << portStr.toLocal8Bit().constData()
                 << "' given." << endl;
            return Error::CommandLineArgs;
        }
    }

    Settings settings;

    // respect overrides from command line
    if (!traceFile.isEmpty())
        settings.setDatabaseFile(traceFile);
    if (portOverride != -1 )
        settings.setServerPort(portOverride);

    MainWindow mw(&settings);

    if (!settings.databaseFile().isEmpty()) {
        QString errMsg;
        if (!mw.setDatabase(settings.databaseFile(), &errMsg)) {
            QMessageBox::critical(0, "Database error",
                                  errMsg);
            return Error::Open;
        }
    }

    settings.restoreSession();

    mw.show();

    int statusCode = a.exec();

    settings.saveSession();

    if (!settings.save()) {
        cerr << "Failed to save GUI settings." << endl;
    }

    return statusCode;
}
