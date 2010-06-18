#include "tracelib.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

static unsigned short g_verbosity = 1;
static TraceLib_Entry_Handler g_handlerFn = &tracelib_entry_handler_null;

void tracelib_set_entry_handler(TraceLib_Entry_Handler handlerFn)
{
    assert(handlerFn != 0);
    g_handlerFn = handlerFn;
}

void tracelib_set_verbosity(unsigned short verbosity)
{
    g_verbosity = verbosity;
}

void tracelib_add_entry(unsigned short verbosity, const char *fn, unsigned int lineno, const char *function)
{
    if (verbosity <= g_verbosity) {
        g_handlerFn(fn, lineno, function);
    }
}

void tracelib_entry_handler_null(const char *fn, unsigned int lineno, const char *function)
{
    (void)fn;
    (void)lineno;
    (void)function;
}

void tracelib_entry_handler_stdout(const char *fn, unsigned int lineno, const char *function)
{
    char timestamp[ 64 ];
    time_t t = time( NULL );
    strftime( timestamp, sizeof( timestamp ), "%d.%m.%Y %H:%M:%S:", localtime( &t) );
    printf( "%s %s:%d: %s\n", timestamp, fn, lineno, function );
}

