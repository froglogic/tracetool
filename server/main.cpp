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

#include "server.h"

#include "database.h"
#include "../hooklib/tracelib_config.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>

#include <iostream>
#include <string>

#ifdef Q_OS_WIN32
#  include <windows.h>
#endif

namespace Error
{
    const int None = 0;
    const int CommandLineArgs = 1;
    const int Database = 2;
}

using namespace std;

static void printUsage(const string &app)
{
    cout << "Usage: " << app << " --help" << endl
         << "       " << app << " [--port <port> [--guiport <port>]] <.trace-file>" << endl;
}

#ifdef Q_OS_WIN32
static BOOL WINAPI ctrlCHandler(DWORD eventType)
{
    switch (eventType) {
        case CTRL_C_EVENT:
            cout << "traced: Detected Ctrl+C, shutting down..." << endl;
            break;
        case CTRL_BREAK_EVENT:
            cout << "traced: Detected Ctrl+Break, shutting down..." << endl;
            break;
        case CTRL_CLOSE_EVENT:
            cout << "traced: Detected console closing, shutting down..." << endl;
            break;
        case CTRL_LOGOFF_EVENT:
            cout << "traced: Detected user logging off, shutting down..." << endl;
            break;
        case CTRL_SHUTDOWN_EVENT:
            cout << "traced: Detected system shutdown, shutting down..." << endl;
            break;
    }
    QCoreApplication::quit();
    return TRUE;
}
#endif

int main( int argc, char **argv )
{
#ifdef Q_OS_WIN32
    if (::SetConsoleCtrlHandler(ctrlCHandler, TRUE) == 0) {
        cout << "Warning: failed to install Ctrl+C handler" << endl;
        cout << "Terminating server using Ctrl+C will not cause a clean shutdown." << endl;
    }
#endif

    QCoreApplication app( argc, argv );

    QCommandLineParser opt;
    QCommandLineOption portOption(QStringList() << "p" << "port", "Listening Port for the trace library to connect to.",
                                  "port", QString::number(TRACELIB_DEFAULT_PORT));
    QCommandLineOption guiportOption(QStringList() << "g" << "guiport", "Listening Port for the trace gui to connect to.",
                                     "guiport", QString::number(TRACELIB_DEFAULT_PORT + 1));
    opt.addHelpOption();
    opt.addVersionOption();
    opt.setApplicationDescription("Listens for trace library connections to store trace entries into a database");
    opt.addOption(portOption);
    opt.addOption(guiportOption);
    opt.addPositionalArgument(".trace_file", "Trace database to store the trace entries into");
    opt.process(app);

    if (opt.positionalArguments().isEmpty()) {
        fprintf(stderr, "Missing command line argument.\n");
        opt.showHelp(Error::CommandLineArgs);
    }
    QString traceFile = opt.positionalArguments().at(0);
    QString errMsg;
    if (!Database::isValidFileName(traceFile, &errMsg)) {
        cout << errMsg.toLocal8Bit().constData() << endl;
        return Error::CommandLineArgs;
    }
	bool ok;
    int port = opt.value(portOption).toInt(&ok);
	if (!ok) {
	    cout << "Invalid port number '"
         << opt.value(portOption).toLocal8Bit().constData()
		 << "' given." << endl;
	    return Error::CommandLineArgs;
    }
    int guiport = opt.value(guiportOption).toInt(&ok);
	if (!ok) {
        cout << "Invalid gui port number '"
         << opt.value(guiportOption).toLocal8Bit().constData()
		 << "' given." << endl;
	    return Error::CommandLineArgs;
    }
    if (port == guiport) {
	cout << "Trace port and GUI port have to be different." << endl;
	return Error::CommandLineArgs;
    }

    QSqlDatabase database;
    if (QFile::exists(traceFile)) {
        database = Database::open(traceFile, &errMsg);
    } else {
        database = Database::create(traceFile, &errMsg);
    }
    if (!database.isValid()) {
        cout << "Failed to open log database: "
             << errMsg.toLocal8Bit().constData()
             << endl;
        return Error::Database;
    }

    Server server(traceFile, database, port, guiport);

    return app.exec();
}

