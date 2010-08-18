#include "configuration.h"

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>

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

static string processFullName()
{
    string pn =
#if defined(__GLIBC__)
        program_invocation_name;
#else
        processName();
#endif
    if ( pn.size() == 0 || pn[0] == '/' )
        return pn;

    if ( pn.find( '/' ) != string::npos ) {
        char buf[PATH_MAX + 1];
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
    return pn.substr( 0, pn.rfind( '/' ) ) + "/tracelib.xml";
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

TRACELIB_NAMESPACE_END

