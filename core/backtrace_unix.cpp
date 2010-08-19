#include "backtrace.h"

#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#ifdef __GNUC__
# include <execinfo.h>
# if __GNUC__ - 0 > 2
#  include <cxxabi.h>
# endif
#else
# include <ucontext.h>
# include <dlfcn.h>
# include <demangle.h>
#endif

using namespace std;

TRACELIB_NAMESPACE_BEGIN

static int trace_ref_count;

static pthread_mutex_t trace_mutex;

static char *symbol_buffer;
static size_t symbol_buffer_length;

#if !defined(__GNUC__) && defined(__sun)
static int buildBackTrace( uintptr_t p, int, void *user) {
    std::vector<StackFrame> *trace = (std::vector<StackFrame> *)user;
    StackFrame frame;
    Dl_info info;
    string line;
    if (dladdr((void *)p, &info)) {
        if (!cplus_demangle( info.dli_sname, symbol_buffer, symbol_buffer_length))
            line = symbol_buffer;
        else
            line = info.dli_sname;
        frame.function = info.dli_fname;
        /*TODO extract fields for SUN */
    } else {
        frame.function = "??";
    }
    trace->push_back( frame );
    return 0;
}
#endif

static bool parseLine( const string line, StackFrame *frame )
{
# if defined(__GNUC__) && __GNUC__ - 0 > 2
    size_t pb = line.find( '(' );
    if (pb != string::npos ) {
        int pe = line.find( ')', pb + 1 );
        if ( pe != string::npos ) {
            int pp = line.rfind( '+', pe );

            string sym = line.substr( pb+1, (pp > pb ? pp : pe)-pb-1 );
            int stat;
            abi::__cxa_demangle( sym.c_str(),
                    symbol_buffer,
                    &symbol_buffer_length, &stat );
            if ( stat == 0 )
                frame->function = symbol_buffer;

            if ( pp != string::npos && pp > pb ) {
                string off = line.substr( pp, pp - pb - 1 );
                frame->functionOffset = strtol( off.c_str(), NULL, 16 );
            }
        }
        frame->module = line.substr( 0, pb );

        //TODO
        //frame->sourceFile;

        return true;
    }
# endif
    return false;
}

static void readBacktrace( std::vector<StackFrame> &trace, size_t skip
#ifdef __sun
        ,ucontext_t *context
#endif
)
{
#if __GNUC__
    void *array[50];
    size_t size = backtrace(array, sizeof(array)/sizeof(void*));
    if ( size > 0 && size < sizeof ( array ) / sizeof ( void* ) ) {
        char **strs = backtrace_symbols(array, size);
        if (strs) {
            for (size_t i = skip; i < size; ++i) {
                StackFrame frame;
                if ( parseLine( strs[i], &frame ) ) {
                    trace.push_back( frame );
                } else {
                    fprintf( stderr, "err (%d) %s\n", i, strs[i] );
                    frame.function = "??";
                }
            }
            free(strs);
        }
    }
#elif defined(__sun)
    walkcontext( context, buildBackTrace, (void*)&trace );
#endif
}

BacktraceGenerator::BacktraceGenerator()
{
    if ( !trace_ref_count++ ) {
        pthread_mutex_init( &trace_mutex, NULL );
        symbol_buffer = (char *)malloc( 4096 );
        symbol_buffer_length = 4096;
    }
}

BacktraceGenerator::~BacktraceGenerator()
{
    if ( ! --trace_ref_count ) {
        pthread_mutex_destroy( &trace_mutex );
        free( symbol_buffer );
        symbol_buffer = NULL;
    }
}

Backtrace BacktraceGenerator::generate( size_t skipInnermostFrames )
{
    std::vector<StackFrame> trace;

    pthread_mutex_lock( &trace_mutex );
    readBacktrace( trace, skipInnermostFrames + 2 );
    pthread_mutex_unlock( &trace_mutex );

    return Backtrace( trace );
}

TRACELIB_NAMESPACE_END

