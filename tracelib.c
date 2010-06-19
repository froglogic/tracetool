#include "tracelib.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static tracelib_trace *g_default_trace = 0;
static TraceLib_Entry_Serializer g_serializerFn = &tracelib_null_serializer;
static TraceLib_Output_Writer g_outputFn = &tracelib_stdout_writer;

struct tracelib_trace
{
    unsigned short verbosity;
};

void tracelib_create_trace( tracelib_trace **trace )
{
    assert( trace );
    assert( *trace );
    *trace = (tracelib_trace *)malloc(sizeof(tracelib_trace));
    ( *trace )->verbosity = 1;
}

void tracelib_destroy_trace( tracelib_trace *trace )
{
    assert( trace );
    free( trace );
}

void tracelib_set_default_trace( tracelib_trace *trace )
{
    g_default_trace = trace;
}

tracelib_trace *tracelib_get_default_trace()
{
    return g_default_trace;
}

void tracelib_add_entry(tracelib_trace *trace, unsigned short verbosity, const char *fn, unsigned int lineno, const char *function)
{
    if (verbosity <= trace->verbosity) {
        char buf[ 1024 ]; /* XXX Avoid fixed buffer size */
        size_t bufsize = g_serializerFn(fn, lineno, function, buf, sizeof(buf));
        if (bufsize > 0) {
            g_outputFn(buf, bufsize);
        }
    }
}

void tracelib_set_verbosity(tracelib_trace *trace, unsigned short verbosity)
{
    assert( trace );
    trace->verbosity = verbosity;
}

void tracelib_set_entry_serializer(TraceLib_Entry_Serializer fn)
{
    assert(fn != 0);
    g_serializerFn = fn;
}

void tracelib_set_output_writer(TraceLib_Output_Writer fn)
{
    assert(fn != 0);
    g_outputFn = fn;
}

size_t tracelib_null_serializer(const char *filename,
                                unsigned int lineno,
                                const char *function,
                                char *buf,
                                size_t bufsize)
{
    (void)filename;
    (void)lineno;
    (void)function;
    (void)buf;
    (void)bufsize;
    return 0;
}

void tracelib_null_writer(const char *buf, size_t bufsize)
{
    (void)buf;
    (void)bufsize;
}

size_t tracelib_plaintext_serializer(const char *filename,
                                     unsigned int lineno,
                                     const char *function,
                                     char *buf,
                                     size_t bufsize)
{
    char timestamp[ 64 ];
    time_t t = time( NULL );
    int nwritten;

    strftime( timestamp, sizeof( timestamp ), "%d.%m.%Y %H:%M:%S:", localtime( &t) );

    nwritten = _snprintf(buf, bufsize, "%s %s:%d: %s\n", timestamp, filename, lineno, function);
    if ( nwritten >= 0 && (size_t)nwritten < bufsize ) {
        return (size_t)nwritten;
    }
    return 0;
}

void tracelib_stdout_writer(const char *buf, size_t bufsize)
{
    fprintf( stdout, buf ); /* XXX don't ignore bufsize */
}

