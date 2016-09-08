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
}

static void printHelp(const QString &app)
{
    fprintf(stdout, "%s:\n"
	    "    --help                  Print this help\n"
	    "    --upgrade <database>    Upgrade to current version\n"
	    "    --downgrade <database>  Downgrade to current version\n"
	    "    --accept-data-loss      Accept possible data loss that might occur on downgrades\n"
	    "\n", qPrintable(app));
}

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

static int downgradeDatabase(const QString &downgradeFile)
{
    QString errMsg;
    QSqlDatabase db = Database::openAnyVersion(downgradeFile, &errMsg);
    if (!db.isValid()) {
	fprintf(stderr, "Downgrade error: %s\n", qPrintable(errMsg));
	return Error::Open;
    }
    if (!Database::downgrade(db, &errMsg)) {
	fprintf(stderr, "Downgrade error: %s\n", qPrintable(errMsg));
	return Error::Conversion;
    }
    return Error::None;
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    GetOpt opt;
    bool help, acceptDataLoss;
    QString upgradeFile, downgradeFile;
    opt.addSwitch("help", &help);
    opt.addOption(0, "upgrade", &upgradeFile);
    opt.addOption(0, "downgrade", &downgradeFile);
    opt.addSwitch("accept-data-loss", &acceptDataLoss);
    if (!opt.parse()) {
        fprintf(stderr, "Invalid command line argument. Try --help.\n");
	return Error::CommandLineArgs;
    }


    if (help) {
        printHelp(opt.appName());
        return 0;
    }
    if (!upgradeFile.isNull() && !downgradeFile.isNull()) {
	fprintf(stderr, "Sorry, cannot upgrade and downgrade at the "
		"same time.\n");
	return Error::CommandLineArgs;
    }
    if (!upgradeFile.isNull()) {
	return upgradeDatabase(upgradeFile);
    } else if (!downgradeFile.isNull()) {
	if (!acceptDataLoss) {
	    fprintf(stderr,
"Downgrade operations can result in loss of extra information\n"
"stored in newer versions. Specify the --accept-data-loss switch\n"
"in addition to --downgrade to accept this. It is recommended to\n"
"perform this operation on a copy of the database.\n");
            return Error::CommandLineArgs;
	}
	return downgradeDatabase(downgradeFile);
    } else {
        fprintf(stderr, "Missing command line argument. Try --help.\n");
        return Error::CommandLineArgs;
    }
}
