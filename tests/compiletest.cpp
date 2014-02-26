#include <tracelib/tracelib.h>

#include <string.h>

// Test that TRACELIB_NAMESPACE_* macros work as expected
TRACELIB_NAMESPACE_BEGIN
int someRandomIdentifier;
TRACELIB_NAMESPACE_END
static void testNamespaceMacros()
{
    TRACELIB_NAMESPACE_IDENT(someRandomIdentifier) = 3;
}

// Test that TRACELIB_DEFAULT_PORT is properly convertible to an unsigned short
static void testPortMacro()
{
    const unsigned short port = TRACELIB_DEFAULT_PORT;
}

// Test that TRACELIB_DEFAULT_CONFIGFILE_NAME can be treated as a C string
static void testConfigFileNameMacro()
{
    (void)strlen( TRACELIB_DEFAULT_CONFIGFILE_NAME );
}

// Test all 'trace' macros
static void testTraceMacros()
{
    TRACELIB_TRACE;

    TRACELIB_TRACE_KEY("somekey");

    TRACELIB_TRACE_KEY_MSG("somekey", "somemessage");

    int i;
    TRACELIB_TRACE_KEY_MSG("somekey", "somemessage, with " << TRACELIB_VALUE(v));

    fTrace("somekey") << "this is a message " << i << fValue(i);
}

// Test all 'debug' macros
static void testDebugMacros()
{
    TRACELIB_DEBUG;

    TRACELIB_DEBUG_KEY("somekey");

    TRACELIB_DEBUG_KEY_MSG("somekey", "somemessage");

    int i;
    TRACELIB_DEBUG_KEY_MSG("somekey", "somemessage, with " << TRACELIB_VALUE(v));

    fDebug("somekey") << "this is a message " << i << fValue(i);
}

// Test all 'error' macros
static void testErrorMacros()
{
    TRACELIB_ERROR;

    TRACELIB_ERROR_KEY("somekey");

    TRACELIB_ERROR_KEY_MSG("somekey", "somemessage");

    int i;
    TRACELIB_ERROR_KEY_MSG("somekey", "somemessage, with " << TRACELIB_VALUE(v));

    fError("somekey") << "this is a message " << i << fValue(i);
}

// Test all 'watch' macros
static void testWatchMacros()
{
    TRACELIB_WATCH;

    TRACELIB_WATCH_KEY("somekey");

    TRACELIB_WATCH_KEY_MSG("somekey", "somemessage");

    int i;
    TRACELIB_WATCH_KEY_MSG("somekey", "somemessage, with " << TRACELIB_VAR(v));

    fWatch("somekey") << "this is a message " << i << fVar(i);
}

int main()
{
    testNamespaceMacros();
    testPortMacro();
    testConfigFileNameMacro();
    testTraceMacros();
    testDebugMacros();
    testErrorMacros();
    testWatchMacros();
}

