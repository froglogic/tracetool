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
