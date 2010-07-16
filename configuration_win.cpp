#include "configuration.h"

#include <windows.h>

using namespace Tracelib;
using namespace std;

string Configuration::configurationFileName()
{
    /* According to the MSDN documentation on GetEnvironmentVariable, an
     * environment variable has a maximum size limit of 32767 characters
     * including the terminating null.
     */
    char buf[ 32767 ] = { L'0' };
    if ( ::GetEnvironmentVariable( "TRACELIB_CONFIG_FILE", buf, sizeof( buf ) ) ) {
        return &buf[0];
    }

    static string defaultFileName;
    if ( defaultFileName.empty() ) {
        ::GetModuleFileName( NULL, buf, sizeof( buf ) );
        char *lastSeparator = strrchr( buf, '\\' );
        strcpy( lastSeparator + 1, "tracelib.xml" ); // XXX Guard against buffer overflow
        defaultFileName = &buf[0];
    }
    return defaultFileName;
}

string Configuration::currentProcessName()
{
    static char buf[ 32768 ] = { '\0' };
    static const char *lastSeparator = 0;
    if ( !lastSeparator ) {
        ::GetModuleFileName( NULL, buf, sizeof( buf ) );
        lastSeparator = strrchr( buf, '\\' );
    }
    return lastSeparator + 1;
}

