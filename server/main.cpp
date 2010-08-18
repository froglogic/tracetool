#include "server.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>

static const char *initialDatabaseStatements[] = {
    "CREATE TABLE trace_entry (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                              " pid INTEGER,"
                              " tid INTEGER,"
                              " timestamp DATETIME,"
                              " tracepoint_id INTEGER,"
                              " message TEXT);",
    "CREATE TABLE trace_point (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                              " verbosity INTEGER,"
                              " type INTEGER,"
                              " path_id INTEGER,"
                              " line INTEGER,"
                              " function_id INTEGER,"
                              " UNIQUE(verbosity, type, path_id, line, function_id));",
    "CREATE TABLE function_name (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                " name TEXT,"
                                " UNIQUE(name));",
    "CREATE TABLE path_name (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            " name TEXT,"
                            " UNIQUE(name));",
    "CREATE TABLE variable_value (tracepoint_id INTEGER,"
                                 " name TEXT,"
                                 " value TEXT,"
                                 " UNIQUE(tracepoint_id, name));",
    "CREATE TABLE backtrace (tracepoint_id INTEGER,"
                            " line INTEGER,"
                            " text TEXT);"
};

int main( int argc, char **argv )
{
    QCoreApplication app( argc, argv );

    const QString databaseFileName = "tracedata.dat";

    const bool initializeDatabase = !QFile::exists( databaseFileName );

    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setDatabaseName( databaseFileName );
    if ( !db.open() ) {
        qWarning() << "Failed to open SQL database";
        return 1;
    }

    if ( initializeDatabase ) {
        QSqlQuery query;
        for ( int i = 0; i < sizeof(initialDatabaseStatements) / sizeof(initialDatabaseStatements[0]); ++i ) {
            query.exec( initialDatabaseStatements[i] );
        }
    }

    Server server( &app, &db, 12382 );

    return app.exec();
}

