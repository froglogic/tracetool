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

#include "configuration.h"

#include <windows.h>
#include <assert.h>
#  include <shlobj.h> // SHGetFolderPath
// Fixup outdated windows headers on older MinGW
#  ifndef CSIDL_FLAG_CREATE
#    define CSIDL_FLAG_CREATE 0x8000
#  endif

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

bool Configuration::isAbsolute( const string &filename )
{
    return !filename.empty() && filename.size() > 2 && filename[1] == ':' && filename[2] == '\\';
}

string Configuration::pathSeparator()
{
    return "\\";
}

string Configuration::userHome()
{
    char path[MAX_PATH] = { 0 };
    HRESULT result = SHGetFolderPath( 0, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, path );
    assert( result == S_OK );

    return path;
}

string Configuration::executableName( const string &basename )
{
    return basename + ".exe";
}

TRACELIB_NAMESPACE_END

