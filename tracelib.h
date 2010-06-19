#ifndef TRACELIB_H
#define TRACELIB_H

#include <stddef.h>

#ifdef _MSC_VER
#  define TRACELIB_BEACON(verbosity) tracelib_add_entry((verbosity), __FILE__, __LINE__, __FUNCSIG__ );
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

typedef size_t(*TraceLib_Entry_Serializer)(const char *filename,
                                           unsigned int lineno,
                                           const char *function,
                                           char *buf,
                                           size_t bufsize);
typedef void(*TraceLib_Output_Writer)(const char *buf,
                                      size_t bufsize); 

typedef struct tracelib_trace tracelib_trace;

void TRACELIB_EXPORT tracelib_create_trace( tracelib_trace **trace );
void TRACELIB_EXPORT tracelib_destroy_trace( tracelib_trace *trace );

void TRACELIB_EXPORT tracelib_add_entry(unsigned short verbosity,
                                        const char *fn,
                                        unsigned int lineno,
                                        const char *function);
void TRACELIB_EXPORT tracelib_set_verbosity(unsigned short verbosity);

void TRACELIB_EXPORT tracelib_set_entry_serializer(TraceLib_Entry_Serializer fn);
void TRACELIB_EXPORT tracelib_set_output_writer(TraceLib_Output_Writer fn);

size_t TRACELIB_EXPORT tracelib_null_serializer(const char *filename,
                                                unsigned int lineno,
                                                const char *function,
                                                char *buf,
                                                size_t bufsize);
void TRACELIB_EXPORT tracelib_null_writer(const char *buf,
                                          size_t bufsize);

size_t TRACELIB_EXPORT tracelib_plaintext_serializer(const char *filename,
                                                     unsigned int lineno,
                                                     const char *function,
                                                     char *buf,
                                                     size_t bufsize);
void TRACELIB_EXPORT tracelib_stdout_writer(const char *buf,
                                            size_t bufsize);
#ifdef __cplusplus
}
#endif

#endif // !defined(TRACELIB_H)
