/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "server.h"

#include "database.h"
#include "../hooklib/tracelib_config.h"
#include "../convertdb/getopt.h"

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

    GetOpt opt;
    bool help;
    QString traceFile, portStr, guiPortStr;
    opt.addSwitch("help", &help);
    opt.addOption('p', "port", &portStr);
    opt.addOption('g', "guiport", &guiPortStr);
    opt.addOptionalArgument("trace_file", &traceFile);
    if (!opt.parse()) {
	cout << "Invalid command line argument. Try --help." << endl;
	return Error::CommandLineArgs;
    }

    if (help) {
	printUsage(argv[0]);
	return Error::None;
    }
    if (traceFile.isEmpty()) {
	cout << "Missing trace file argument. Try --help." << endl;
	return Error::CommandLineArgs;
    }
    QString errMsg;
    if (!Database::isValidFileName(traceFile, &errMsg)) {
        cout << errMsg.toLocal8Bit().constData() << endl;
        return Error::CommandLineArgs;
    }
    int port = TRACELIB_DEFAULT_PORT;
    if (!portStr.isEmpty()) {
	bool ok;
	port = portStr.toInt(&ok);
	if (!ok) {
	    cout << "Invalid port number '"
		 << portStr.toLocal8Bit().constData()
		 << "' given." << endl;
	    return Error::CommandLineArgs;
	}
    }
    int guiport = port + 1;
    if (!guiPortStr.isEmpty()) {
	bool ok;
	guiport = guiPortStr.toInt(&ok);
	if (!ok) {
	    cout << "Invalid port number '"
		 << guiPortStr.toLocal8Bit().constData()
		 << "' given." << endl;
	    return Error::CommandLineArgs;
	}
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

