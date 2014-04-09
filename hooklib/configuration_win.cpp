/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "configuration.h"

#include <windows.h>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

/* XXX Consider encoding issues; this function should return UTF-8 encoded
 * strings.
 */
string Configuration::defaultFileName()
{
    /* According to the MSDN documentation on GetEnvironmentVariable, an
     * environment variable has a maximum size limit of 32767 characters
     * including the terminating null.
     */
    char buf[ 32767 ] = { L'0' };
    if ( ::GetEnvironmentVariable( "TRACELIB_CONFIG_FILE", buf, sizeof( buf ) ) ) {
        return std::string( buf );
    }

    static string defaultName;
    if ( defaultName.empty() ) {
        ::GetModuleFileName( NULL, buf, sizeof( buf ) );
        char *lastSeparator = strrchr( buf, '\\' );

        // XXX Guard against buffer overflow
        strcpy( lastSeparator + 1, TRACELIB_DEFAULT_CONFIGFILE_NAME );
        defaultName = &buf[0];
    }
    return defaultName;
}

/* XXX Consider encoding issues; this function should return UTF-8 encoded
 * strings.
 */
string Configuration::currentProcessName()
{
    static const char *lastSeparator = 0;
    if ( !lastSeparator ) {
        static char buf[ 32768 ] = { '\0' };
        ::GetModuleFileName( NULL, buf, sizeof( buf ) );
        lastSeparator = strrchr( buf, '\\' );
    }
    return lastSeparator + 1;
}

TRACELIB_NAMESPACE_END

