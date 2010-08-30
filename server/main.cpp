#include "server.h"

#include "../convertdb/getopt.h"

#include <QCoreApplication>
#include <QFile>

#include <iostream>
#include <string>

namespace Error
{
    const int None = 0;
    const int CommandLineArgs = 1;
    const int Open = 2;
    const int Conversion = 3;
};

using namespace std;

static void printUsage(const string &app)
{
    cout << "Usage: " << app << " --help" << endl
         << "       " << app << " [--port <port>] <.trace-file>" << endl;
}

int main( int argc, char **argv )
{
    QCoreApplication app( argc, argv );

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
    if (traceFile.isEmpty()) {
	cout << "Missing trace file argument. Try --help." << endl;
	return Error::CommandLineArgs;
    }
    if (!traceFile.endsWith(".trace", Qt::CaseInsensitive)) {
	// ### Too picky? No real technical reason so far. But
	// ### helps the IDE and who knows what else in the
	// ### future. Click a file type association that allows
	// ### opening traces by double-click.
	cout << "Trace file is expected to have a "
	     << ".trace suffix. " << endl;
	return Error::CommandLineArgs;
    }
    int port = 12382; // ### use globally configurable default
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

    Server server(traceFile, port);

    return app.exec();
}

