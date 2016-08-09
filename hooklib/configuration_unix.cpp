/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "configuration.h"

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <cassert>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(__APPLE__)
#  include <mach-o/dyld.h>
#  include <sys/param.h>
#  include <vector>
#endif

using namespace std;

TRACELIB_NAMESPACE_BEGIN

#if defined(__GLIBC__)
extern "C" {
    extern char* __progname;
    extern char* __progname_full;
    extern char *program_invocation_name;
    extern char *program_invocation_short_name;
}
#endif

static const char *processName()
{
#if defined(__GLIBC__)
    return program_invocation_short_name;
#elif defined(__FreeBSD__) || defined(__APPLE__)
    return getprogname();
#elif defined(__sun)
    return getexecname();
#else
    // XXX implement
    assert( !"currentProcessName() not implemented" );
    return NULL;
#endif
}

#if defined(__APPLE__)
string macProcessPath()
{
    uint32_t size = MAXPATHLEN;
    std::vector<char> buf( size );
    int retVal = _NSGetExecutablePath( &buf[0], &size );
    if( retVal == -1 ) {
        buf.resize( size );
        retVal = _NSGetExecutablePath( &buf[0], &size );
    }
    if( retVal == 0 ) {
        char *tmp = realpath( &buf[0], 0 );
        std::string abspath( tmp );
        free( tmp );
        return abspath;
    }
    // Fallback in case the above fails
    return processName();
}
#endif
string processFullName()
{
    string pn =
#if defined(__GLIBC__)
        program_invocation_name;
#elif defined(__APPLE__)
        macProcessPath();
#else
        processName();
#endif
    if ( pn.size() == 0 || pn[0] == '/' )
        return pn;

    if ( pn.find( '/' ) != string::npos ) {
        char buf[PATH_MAX + 1] = { 0 }; // make valgrind happy

        if ( getcwd( buf, PATH_MAX ) &&
                realpath( ( string( buf ) + '/' + pn ).c_str(), buf ) ) {
            return buf;
        }
    }

    struct stat file_stat;
    string path = getenv( "PATH" );
    size_t sep0 = 0;
    size_t sep1 = path.find( ':' );
    do {
        string file = path.substr( sep0, sep1 - sep0 ) + '/' + pn;
        if ( stat( file.c_str(), &file_stat ) == 0 &&
                S_ISREG( file_stat.st_mode ) )
            return file;
        sep0 = sep1;
        if ( sep0 == string::npos )
            break;
        sep1 = path.find( ':', ++sep0 );
    } while ( true );

    return "";
}

/* XXX Consider encoding issues; this function should return UTF-8 encoded
 * strings.
 */
string Configuration::defaultFileName()
{
    const char *env = getenv( "TRACELIB_CONFIG_FILE" );
    if ( env )
        return env;

    string pn = processFullName();
    return pn.substr( 0, pn.rfind( '/' ) ) + "/" TRACELIB_DEFAULT_CONFIGFILE_NAME;
}

/* XXX Consider encoding issues; this function should return UTF-8 encoded
 * strings.
 */
string Configuration::currentProcessName()
{
    const char *pn = processName();
    const char *s = strrchr( pn, '/');
    if ( s )
        return s + 1;
    return pn;
}

bool Configuration::isAbsolute( const string &filename )
{
    return !filename.empty() && filename[0] == '/';
}

string Configuration::pathSeparator()
{
    return "/";
}

string Configuration::userHome()
{
    passwd *userdata = getpwuid( getuid() );
    return userdata->pw_dir;
}

string Configuration::executableName( const string &basename )
{
    return basename;
}

TRACELIB_NAMESPACE_END

