#include "configuration.h"

#include <cassert>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

/* XXX Consider encoding issues; this function should return UTF-8 encoded
 * strings.
 */
string Configuration::defaultFileName()
{
    // XXX implement
    assert( !"defaultFileName() not implemented." );
    return string();
}

/* XXX Consider encoding issues; this function should return UTF-8 encoded
 * strings.
 */
string Configuration::currentProcessName()
{
    // XXX implement
    assert( !"currentProcessName() not implemented" );
    return string();
}

TRACELIB_NAMESPACE_END

