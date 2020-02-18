/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2013-2016 froglogic GmbH
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

#include "../hooklib/tracelib.h"
#include "../server/xmlcontenthandler.h"
#include "../server/databasefeeder.h"
#include "config.h"

#include <cstdio>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QSqlDatabase>

namespace Error
{
    const int None = 0;
    const int CommandLineArgs = 1;
    const int Open = 2;
    const int File = 3;
    const int Transformation = 4;
}

static bool fromXml( QSqlDatabase &db, QFile &input, QString *errMsg )
{
    DatabaseFeeder feeder( db );
    XmlContentHandler xmlparser(&feeder );
    xmlparser.addData( "<toplevel_trace_element>" );
    while( !input.atEnd() ) {
        try {
            xmlparser.addData( input.read( 1 << 16 ) );
            xmlparser.continueParsing();
        } catch( const SQLTransactionException &ex ) {
            *errMsg = "Database error: " + QString::fromLatin1( ex.what() ) + ", driver message: " + ex.driverMessage() + "(" + QString::number(ex.driverCode()) + ")";
            return false;
        } catch( const XmlParseException &ex ) {
            *errMsg = "XML error: " + QString::fromLatin1( ex.what() ) + ", driver message: " + ex.parserMessage() + "(" + QString::number(ex.parserCode()) + ")";
            return false;
        }
    }
    return true;
}

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );
    a.setApplicationVersion(QLatin1String(TRACELIB_VERSION_STR));

    QCommandLineParser opt;
    QCommandLineOption inputOption(QStringList() << "i" << "input", "XML input file to read from, if not specified reads from stdin", "file");
    opt.setApplicationDescription("Converts xml files into trace databases.");
    opt.addHelpOption();
    opt.addVersionOption();
    opt.addOption(inputOption);
    opt.addPositionalArgument(".trace-file", "Trace database output file to write into (.trace suffix will be appended if missing).");
    opt.process(a);

    if ( opt.positionalArguments().isEmpty() ) {
        fprintf(stderr, "Missing output trace database filename\n");
        opt.showHelp(Error::CommandLineArgs);
    }

    QString traceFile = opt.positionalArguments().at(0);

    QString errMsg;
    QSqlDatabase db;
    if (!traceFile.endsWith(".trace")) {
        traceFile += ".trace";
    }
    if (QFile::exists(traceFile)) {
        db = Database::open(traceFile, &errMsg);
    } else {
        db = Database::create(traceFile, &errMsg);
    }
    if (!db.isValid()) {
        fprintf( stderr, "Failed to open output trace database %s: %s\n", qPrintable( traceFile ), qPrintable( errMsg ));
        return Error::Open;
    }

    QFile input;
    if ( !opt.isSet( inputOption ) ) {
        input.open( stdin, QIODevice::ReadOnly );
    } else {
        QString xmlFile = opt.value( inputOption );
        input.setFileName( xmlFile );
        if (!input.open( QIODevice::ReadOnly )) {
            fprintf( stderr, "File '%s' cannot be opened for writing.\n", qPrintable( xmlFile ));
            return Error::File;
        }
    }

    if (!fromXml( db, input, &errMsg )) {
        fprintf( stderr, "Transformation error: %s\n", qPrintable( errMsg ));
        return Error::Transformation;
    }
    input.close();
    return Error::None;
}
