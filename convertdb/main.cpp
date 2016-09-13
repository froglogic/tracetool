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


#include "../server/database.h"

#include <cstdio>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QSqlDatabase>

namespace Error
{
    const int None = 0;
    const int CommandLineArgs = 1;
    const int Open = 2;
    const int Conversion = 3;
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

    QCommandLineParser opt;
    QCommandLineOption acceptDataLoss("accept-data-loss", "Accept possible data loss that might occur on downgrades");
    QCommandLineOption upgradeFile("upgrade", "Upgrade to current version", "database");
    QCommandLineOption downgradeFile("downgrade", "Downgrade to current version", "database");
    opt.setApplicationDescription("Converts trace databases between different versions");
    opt.addOption(acceptDataLoss);
    opt.addOption(upgradeFile);
    opt.addOption(downgradeFile);
    opt.addHelpOption();
    opt.addVersionOption();
    opt.process(a);
    if (opt.isSet(upgradeFile) && opt.isSet(upgradeFile)) {
	fprintf(stderr, "Sorry, cannot upgrade and downgrade at the "
		"same time.\n");
	return Error::CommandLineArgs;
    }
    if (opt.isSet(upgradeFile)) {
    return upgradeDatabase(opt.value(upgradeFile));
    } else if (opt.isSet(downgradeFile)) {
    if (!opt.isSet(acceptDataLoss)) {
	    fprintf(stderr,
"Downgrade operations can result in loss of extra information\n"
"stored in newer versions. Specify the --accept-data-loss switch\n"
"in addition to --downgrade to accept this. It is recommended to\n"
"perform this operation on a copy of the database.\n");
            return Error::CommandLineArgs;
	}
    return downgradeDatabase(opt.value(downgradeFile));
    } else {
        fprintf(stderr, "Missing command line argument.\n");
        opt.showHelp(Error::CommandLineArgs);
    }
}
