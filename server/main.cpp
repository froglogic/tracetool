#include "server.h"

#include <QCoreApplication>

int main( int argc, char **argv )
{
    QCoreApplication app( argc, argv );

    Server server( &app, "tracedata.dat", 12382 );

    return app.exec();
}

