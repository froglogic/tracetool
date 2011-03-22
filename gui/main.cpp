/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include <QApplication>
#include <QFile>
#include <QMessageBox>

#include "mainwindow.h"
#include "settings.h"
#include "../convertdb/getopt.h"
#include "../server/database.h"
#ifdef Q_OS_WIN
#  include "jobobject.h"
#endif

#include <iostream>
#include <string>

namespace Error
{
    const int None = 0;
    const int CommandLineArgs = 1;
    const int Open = 2;
}

using namespace std;

static void printUsage(const string &app)
{
    cout << "Usage: " << app << " --help" << endl
         << "       " << app << " <.trace-file>" << endl;
}

int main(int argc, char **argv)
{
#ifdef Q_OS_WIN
    JobObject job;
    job.assignProcess(::GetCurrentProcess());
#endif

    QApplication a(argc, argv);

    GetOpt opt;
    bool help;
    QString traceFile;
    opt.addSwitch("help", &help);
    opt.addOptionalArgument("trace_file", &traceFile);
    if (!opt.parse()) {
        cout << "Invalid command line argument. Try --help." << endl;
        return Error::CommandLineArgs;
    }

    if (help) {
        printUsage(argv[0]);
        return Error::None;
    }
    QString errMsg;
    if (!traceFile.isEmpty() &&
        !Database::isValidFileName(traceFile, &errMsg)) {
        cout << errMsg.toLocal8Bit().constData() << endl;
        return Error::CommandLineArgs;
    }

    Settings settings;

    // respect overrides from command line
    if (!traceFile.isEmpty())
        settings.setDatabaseFile(traceFile);

#ifdef Q_OS_WIN
    MainWindow mw(&settings, &job);
#else
    MainWindow mw(&settings);
#endif
    settings.restoreSession();

    // ### ugly
    mw.postRestore();

    if (!traceFile.isEmpty()) {
        if (!mw.setDatabase(traceFile, &errMsg)) {
            QMessageBox::critical(0, "Database error",
                                  errMsg);
            return Error::Open;
        }
    } else {
        mw.connectToServer();
    }

    mw.show();

    int statusCode = a.exec();

    settings.saveSession();

    if (!settings.save()) {
        cerr << "Failed to save GUI settings." << endl;
    }

    return statusCode;
}
