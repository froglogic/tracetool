#include "getopt.h"

#include "../server/database.h"

#include <cstdio>
#include <QCoreApplication>
#include <QSqlDatabase>

namespace Error
{
    const int None = 0;
    const int CommandLineArgs = 1;
    const int Open = 2;
    const int Conversion = 3;
};

static void printHelp(const QString &app)
{
    fprintf(stdout, "%s:\n"
	    "    --help                Print this help\n"
	    "    --upgrade <database>  Upgrade to current version"
	    "\n", qPrintable(app));
}

#include <QSqlQuery>

static int upgradeDatabase(const QString &upgradeFile)
{
    QString errMsg;
    QSqlDatabase db = Database::openAnyVersion(upgradeFile, &errMsg);
    if (!db.isValid()) {
	fprintf(stderr, "Upgrade error: %s\n", qPrintable(errMsg));
	return Error::Open;
    }
    if (!Database::upgrade(db, &errMsg)) {
	fprintf(stderr, "Upgrade error: %s\n", qPrintable(errMsg));
	return Error::Conversion;
    }
    return Error::None;
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    GetOpt opt;
    bool help;
    QString upgradeFile;
    opt.addSwitch("help", &help);
    opt.addOption('u', "upgrade", &upgradeFile);
    if (!opt.parse()) {
        fprintf(stderr, "Invalid command line argument. Try --help.\n");
	return Error::CommandLineArgs;
    }


    if (help) {
        printHelp(opt.appName());
        return 0;
    } else if (!upgradeFile.isNull()) {
	return upgradeDatabase(upgradeFile);
    } else {
        fprintf(stderr, "Missing command line argument. Try --help.\n");
        return Error::CommandLineArgs;
    }


    return 0;
}
