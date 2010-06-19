#ifndef TRACELIB_H
#define TRACELIB_H

#include <stddef.h>

#ifdef _MSC_VER
#  define TRACELIB_BEACON(verbosity) tracelib_trace_add_entry(tracelib_get_default_trace(), (verbosity), __FILE__, __LINE__, __FUNCSIG__ );
#  ifdef TRACELIB_MAKEDLL
#    define TRACELIB_EXPORT __declspec(dllexport)
#  else
#    define TRACELIB_EXPORT __declspec(dllimport)
#  endif
#else
#  error "Unsupported compiler!"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t(*tracelib_entry_serializer_fn)(void *data,
                                              const char *filename,
                                              unsigned int lineno,
                                              const char *function,
                                              char *buf,
                                              size_t bufsize);
typedef void(*tracelib_output_writer_fn)(void *data,
                                         const char *buf,
                                         size_t bufsize); 

typedef struct tracelib_trace tracelib_trace;

TRACELIB_EXPORT void tracelib_create_trace( tracelib_trace **trace );
TRACELIB_EXPORT void tracelib_destroy_trace( tracelib_trace *trace );

TRACELIB_EXPORT void tracelib_set_default_trace( tracelib_trace *trace );
TRACELIB_EXPORT tracelib_trace *tracelib_get_default_trace();

TRACELIB_EXPORT void tracelib_trace_add_entry(tracelib_trace *trace,
                                              unsigned short verbosity,
                                              const char *fn,
                                              unsigned int lineno,
                                              const char *function);
TRACELIB_EXPORT void tracelib_trace_set_verbosity(tracelib_trace *trace,
                                                  unsigned short verbosity);
TRACELIB_EXPORT void tracelib_trace_set_entry_serializer(tracelib_trace *trace,
                                                         tracelib_entry_serializer_fn fn,
                                                         void *data);
TRACELIB_EXPORT void tracelib_trace_set_output_writer(tracelib_trace *trace,
                                                      tracelib_output_writer_fn fn,
                                                      void *data);

TRACELIB_EXPORT size_t tracelib_null_serializer(void *data,
                                                const char *filename,
                                                unsigned int lineno,
                                                const char *function,
                                                char *buf,
                                                size_t bufsize);
TRACELIB_EXPORT void tracelib_null_writer(void *data,
                                          const char *buf,
                                          size_t bufsize);

typedef struct {
    int show_timestamp;
} tracelib_plaintext_serializer_args;

size_t TRACELIB_EXPORT tracelib_plaintext_serializer(void *data,
                                                     const char *filename,
                                                     unsigned int lineno,
                                                     const char *function,
                                                     char *buf,
                                                     size_t bufsize);
TRACELIB_EXPORT void tracelib_stdout_writer(void *data,
                                            const char *buf,
                                            size_t bufsize);
#ifdef __cplusplus
}
#endif

#endif // !defined(TRACELIB_H)
