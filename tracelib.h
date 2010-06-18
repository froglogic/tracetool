#ifndef TRACELIB_H
#define TRACELIB_H

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

typedef void(*TraceLib_Entry_Handler)(const char *filename,
                                      unsigned int lineno,
                                      const char *function);

void TRACELIB_EXPORT tracelib_add_entry(unsigned short verbosity,
                                        const char *fn,
                                        unsigned int lineno,
                                        const char *function);
void TRACELIB_EXPORT tracelib_set_verbosity(unsigned short verbosity);

void TRACELIB_EXPORT tracelib_set_entry_handler(TraceLib_Entry_Handler handlerFn);

void TRACELIB_EXPORT tracelib_entry_handler_null(const char *filename,
                                                 unsigned int lineno,
                                                 const char *function);
void TRACELIB_EXPORT tracelib_entry_handler_stdout(const char *filename,
                                                   unsigned int lineno,
                                                   const char *function);
#ifdef __cplusplus
}
#endif

#endif // !defined(TRACELIB_H)
