#include "tracelib.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#  include <io.h> // for write()
#  define write _write
#endif

static tracelib_trace *g_default_trace = 0;

struct tracelib_trace
{
    unsigned short verbosity;
    tracelib_entry_serializer_fn serializerFn;
    void *serializer_data;
    tracelib_output_writer_fn outputFn;
    void *output_data;
};

void tracelib_create_trace(tracelib_trace **trace)
{
    assert(trace);
    *trace = (tracelib_trace *)malloc(sizeof(tracelib_trace));
    (*trace)->verbosity = 1;
    (*trace)->serializerFn = &tracelib_null_serializer;
    (*trace)->serializer_data = 0;
    (*trace)->outputFn = &tracelib_null_writer;
    (*trace)->output_data = 0;
}

void tracelib_destroy_trace(tracelib_trace *trace)
{
    assert(trace);
    free(trace);
}

void tracelib_set_default_trace(tracelib_trace *trace)
{
    g_default_trace = trace;
}

tracelib_trace *tracelib_get_default_trace()
{
    return g_default_trace;
}

void tracelib_trace_add_entry(tracelib_trace *trace, unsigned short verbosity, const char *fn, unsigned int lineno, const char *function)
{
    assert(trace);
    if (verbosity <= trace->verbosity) {
        char buf[ 1024 ]; /* XXX Avoid fixed buffer size */
        size_t bufsize = trace->serializerFn(trace->serializer_data, fn, lineno, function, buf, sizeof(buf));
        if (bufsize > 0) {
            trace->outputFn(trace->output_data, buf, bufsize);
        }
    }
}

void tracelib_trace_set_verbosity(tracelib_trace *trace, unsigned short verbosity)
{
    assert(trace);
    trace->verbosity = verbosity;
}

void tracelib_trace_set_entry_serializer(tracelib_trace *trace,
                                         tracelib_entry_serializer_fn fn,
                                         void *data)
{
    assert(trace);
    assert(fn);
    trace->serializerFn = fn;
    trace->serializer_data = data;
}

void tracelib_trace_set_output_writer(tracelib_trace *trace,
                                      tracelib_output_writer_fn fn,
                                      void *data)
{
    assert(trace);
    assert(fn);
    trace->outputFn = fn;
    trace->output_data = data;
}

size_t tracelib_null_serializer(void *data,
                                const char *filename,
                                unsigned int lineno,
                                const char *function,
                                char *buf,
                                size_t bufsize)
{
    (void)data;
    (void)filename;
    (void)lineno;
    (void)function;
    (void)buf;
    (void)bufsize;
    return 0;
}

void tracelib_null_writer(void *data, const char *buf, size_t bufsize)
{
    (void)data;
    (void)buf;
    (void)bufsize;
}

size_t tracelib_plaintext_serializer(void *data,
                                     const char *filename,
                                     unsigned int lineno,
                                     const char *function,
                                     char *buf,
                                     size_t bufsize)
{
    int nwritten;
    char timestamp[64] = { '\0' };

    tracelib_plaintext_serializer_args *args = (tracelib_plaintext_serializer_args *)data;
    if (args && args->show_timestamp) {
        time_t t = time(NULL);
        strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S: ", localtime(&t));
    }

    nwritten = _snprintf(buf, bufsize, "%s%s:%d: %s\n", timestamp, filename, lineno, function);
    if (nwritten >= 0 && (size_t)nwritten < bufsize) {
        return (size_t)nwritten;
    }
    return 0;
}

void tracelib_stdout_writer(void *data, const char *buf, size_t bufsize)
{
    fprintf(stdout, buf); /* XXX don't ignore bufsize */
}

void tracelib_file_writer(void *data, const char *buf, size_t bufsize)
{
    int nwritten = 0;
    tracelib_file_writer_args *args = (tracelib_file_writer_args *)data;
    assert(args);

    // XXX Error handling!
    while (bufsize > 0 && (nwritten = write(args->fd, buf + nwritten, bufsize)) != -1) {
        bufsize -= nwritten;
    }
}

