/**********************************************************************
** Copyright ( C ) 2013 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "../convertdb/getopt.h"

#include "../hooklib/tracelib.h"
#include "../server/xmlcontenthandler.h"
#include "../server/databasefeeder.h"

#include <cstdio>
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

static void printHelp( const QString &app )
{
    fprintf( stdout, "Usage: %s [options] DATABASE\n"
            "Options:\n"
            "  -o, --output FILE   Writes XML to FILE\n"
	    "  --help              Print this help\n"
        "\n", qPrintable( app ));
}

static bool fromXml( QSqlDatabase &db, QFile &input, QString *errMsg )
{
    DatabaseFeeder feeder( db );
    XmlContentHandler xmlparser(&feeder );
    xmlparser.addData( "<toplevel_trace_element>" );
    while( !input.atEnd() ) {
        try {
            xmlparser.addData( input.read( 2^18 ) );
            xmlparser.continueParsing();
        } catch( const SQLTransactionException &ex ) {
            *errMsg = "Database error: " + QString::fromLatin1( ex.what() ) + ", driver message: " + ex.driverMessage() + "(" + QString::number(ex.driverCode()) + ")";
            return false;
        }
    }
    return true;
}

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );

    GetOpt opt;
    bool help;
    QString traceFile, xmlFile;
    opt.addSwitch("help", &help );
    opt.addOption('o', "output", &traceFile );
    opt.addArgument("xmlTrace", &xmlFile );
    if (!opt.parse()) {
        if ( help ) {
            //TODO: bad, since GetOpt.parse() already prints
            //'Lacking required argument' warning
            printHelp( opt.appName());
            return Error::None;
        }
        fprintf( stderr, "Invalid command line argument. Try --help.\n");
        return Error::CommandLineArgs;
    }

    if ( help ) {
        printHelp( opt.appName());
        return Error::None;
    }

    QString errMsg;
    QSqlDatabase db;
    if (QFile::exists(traceFile)) {
        db = Database::open(traceFile, &errMsg);
    } else {
        db = Database::create(traceFile, &errMsg);
    }
    if (!db.isValid()) {
        fprintf( stderr, "Open error: %s\n", qPrintable( errMsg ));
        return Error::Open;
    }

    QFile input;
    if ( xmlFile.isNull()) {
        input.open( stdin, QIODevice::ReadOnly );
    } else {
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
