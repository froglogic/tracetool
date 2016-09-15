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

#include <QCommandLineParser>
#include <QApplication>
#include <QFile>
#include <QMessageBox>

#include "config.h"
#include "mainwindow.h"
#include "settings.h"
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

int main(int argc, char **argv)
{
#ifdef Q_OS_WIN
    JobObject job;
    job.assignProcess(::GetCurrentProcess());
#endif

    QApplication a(argc, argv);
    a.setApplicationVersion(QLatin1String(TRACELIB_VERSION_STR));

    QCommandLineParser opt;
    opt.setApplicationDescription("GUI for analyzing trace files");
    opt.addHelpOption();
    opt.addVersionOption();
    opt.addPositionalArgument(".trace-file", "Trace file to load");
    opt.process(a);
    
    QStringList positionalArguments = opt.positionalArguments();
    QString traceFile;
    if ( positionalArguments.size() > 0 ) {
        traceFile = positionalArguments.at(0);
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
