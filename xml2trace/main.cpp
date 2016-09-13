/**********************************************************************
** Copyright ( C ) 2013 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "../hooklib/tracelib.h"
#include "../server/xmlcontenthandler.h"
#include "../server/databasefeeder.h"

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

static void printHelp( const QString &app )
{
    fprintf( stdout, "Usage: %s [--help | -o TRACEDBFILE [XMLFILE]]\n"
            "Options:\n"
            "  -o, --output FILE   Writes trace database to FILE\n"
	    "  --help              Print this help\n"
            "\n"
            "If the XMLFILE argument is omitted the xml trace log should be passed\n"
            "on the standard input channel\n"
        "\n", qPrintable( app ));
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
        }
    }
    return true;
}

int main( int argc, char **argv )
{
    QCoreApplication a( argc, argv );

    QCommandLineParser opt;
    QCommandLineOption inputOption(QStringList() << "i" << "input", "XML input file to read from, if not specified reads from stdin", "file");
    opt.setApplicationDescription("Converts xml files into trace databases.");
    opt.addHelpOption();
    opt.addVersionOption();
    opt.addOption(inputOption);
    opt.addPositionalArgument(".trace-file", "Trace database output file to write into.");
    opt.process(a);

    if ( opt.positionalArguments().isEmpty() ) {
        fprintf(stderr, "Missing output trace database filename\n");
        opt.showHelp(Error::CommandLineArgs);
    }

    QString traceFile = opt.positionalArguments().at(1);

    QString errMsg;
    QSqlDatabase db;
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
