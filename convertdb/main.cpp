#include "getopt.h"

#include <cstdio>
#include <QCoreApplication>

static void printHelp(const QString &app)
{
    fprintf(stdout, "%s: --help    print help\n", qPrintable(app));
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    GetOpt opt;
    bool help;
    opt.addSwitch("help", &help);
    if (!opt.parse()) {
        fprintf(stderr, "Invalid command line argument. Try --help.\n");
	return 1;
    }


    if (help) {
        printHelp(opt.appName());
        return 0;
    } else {
        fprintf(stderr, "Missing command line argument. Try --help.\n");
        return 1;
    }


    return 0;
}
