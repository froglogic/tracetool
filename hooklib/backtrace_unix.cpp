/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "backtrace.h"

#include <config.h>

#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#if defined(__GNUC__) && defined(HAVE_EXECINFO_H)
# include <execinfo.h>
# if __GNUC__ - 0 > 2
#  include <cxxabi.h>
# endif
#else
# include <ucontext.h>
# include <dlfcn.h>
#endif
#include <dlfcn.h>
#if HAVE_BFD_H && HAVE_DEMANGLE_H
# include <bfd.h>
# include <demangle.h>
#endif

using namespace std;

TRACELIB_NAMESPACE_BEGIN

static int trace_ref_count;

static pthread_mutex_t trace_mutex;

static char *symbol_buffer;
static size_t symbol_buffer_length;

#if HAVE_BFD_H && HAVE_DEMANGLE_H
static bfd *self_bfd;
static asymbol **self_symbols;
#endif

extern string processFullName();

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

#if HAVE_BFD_H && HAVE_DEMANGLE_H

struct BfdSymbol {
    bfd_vma pc;
    bool found;
    const char *filename;
    unsigned int linenr;
    const char *functionname;
    unsigned int offset;
};

static void findAddressInSection( bfd *abfd, asection *section, void *data )
{
    BfdSymbol *bfd_sym = (BfdSymbol *)data;

    if ( bfd_sym->found )
        return;

    if ( ( bfd_get_section_flags( abfd, section ) & SEC_ALLOC) == 0 )
        return;

    bfd_vma vma = bfd_get_section_vma( abfd, section );
    if ( bfd_sym->pc < vma )
        return;

    bfd_size_type size = bfd_get_section_size( section );
    if ( bfd_sym->pc >= vma + size )
        return;

    bfd_sym->offset = bfd_sym->pc - vma;
    bfd_sym->found = bfd_find_nearest_line( abfd,
            section, self_symbols,  bfd_sym->pc - vma,
            &bfd_sym->filename, &bfd_sym->functionname, &bfd_sym->linenr );
}

static bool bfdAddressInfo( bfd_vma addr, StackFrame *frame )
{
    BfdSymbol bfd_sym;
    bfd_sym.pc = addr; //bfd_scan_vma( addr, NULL, 16 );
    bfd_sym.found = false;
    bfd_map_over_sections( self_bfd, findAddressInSection, &bfd_sym );
    if ( bfd_sym.found ) {
        if ( bfd_sym.functionname ) {
            char *demangle = bfd_demangle( self_bfd,
                   bfd_sym.functionname, DMGL_ANSI | DMGL_PARAMS);
            if ( demangle ) {
                frame->function = demangle;
                free( demangle );
            } else {
                frame->function = bfd_sym.functionname;
            }
        } else {
            frame->function = "??";
        }
        if ( bfd_sym.filename )
            frame->sourceFile = bfd_sym.filename;
        frame->lineNumber = bfd_sym.linenr;
        frame->functionOffset = bfd_sym.offset;
    }

    Dl_info dl_info;
    if ( dladdr( (void*)addr, &dl_info ) ) {
        frame->module = dl_info.dli_fname;
        if ( !bfd_sym.found ) {
            char buf[48];
            snprintf( buf, sizeof ( buf ), "[%p + %p]",
                    dl_info.dli_fbase, (char*)addr - (char*)dl_info.dli_fbase );
            frame->function = buf;
        }
    } else if ( !bfd_sym.found ) {
        char buf[32];
        snprintf( buf, sizeof ( buf ), "[%p]", (void*)addr );
        frame->function = buf;
    }

    return true;
}
#endif

#if defined(__GNUC__) && defined(HAVE_EXECINFO_H)
static bool parseLine( const string line, StackFrame *frame )
{
# if __GNUC__ - 0 > 2
    size_t pb = line.find( '(' );
    if (pb != string::npos ) {
        size_t pe = line.find( ')', pb + 1 );
        if ( pe != string::npos ) {
            size_t pp = line.rfind( '+', pe );
            if ( pp < pb )
                pp = string::npos;

            string sym = line.substr( pb+1, (pp == string::npos ? pe : pp)-pb-1 );
            int stat;
            abi::__cxa_demangle( sym.c_str(),
                    symbol_buffer,
                    &symbol_buffer_length, &stat );
            if ( stat == 0 )
                frame->function = symbol_buffer;
            else
                frame->function = sym;

            if ( pp != string::npos ) {
                string off = line.substr( pp, pe - pp );
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
#endif

static void readBacktrace( std::vector<StackFrame> &trace, size_t skip
#ifdef __sun
        ,ucontext_t *context
#endif
)
{
#if defined(__GNUC__) && defined(HAVE_EXECINFO_H)
    void *array[50];
    size_t size = backtrace(array, sizeof(array)/sizeof(void*));
    if ( size > 0 && size < sizeof ( array ) / sizeof ( void* ) ) {
#if HAVE_BFD_H && HAVE_DEMANGLE_H
        if ( self_symbols ) {
            for (size_t i = skip; i < size; ++i) {
                StackFrame frame;
                if ( !bfdAddressInfo( (bfd_vma)array[i], &frame ) ) {
                    fprintf( stderr, "err (%d) %p\n", i, array[i] );
                    frame.function = "??";
                }
                trace.push_back( frame );
            }
        } else {
#endif
            char **strs = backtrace_symbols(array, size);
            if (strs) {
                for (size_t i = skip; i < size; ++i) {
                    StackFrame frame;
                    if ( !parseLine( strs[i], &frame ) ) {
                        fprintf( stderr, "err (%d) %s\n", int(i), strs[i] );
                        frame.function = "??";
                    }
                    trace.push_back( frame );
                }
                free(strs);
            }
#if HAVE_BFD_H && HAVE_DEMANGLE_H
        }
#endif
    }
#elif defined(__sun)
    walkcontext( context, buildBackTrace, (void*)&trace );
#endif
}

static void setupSymbolTable()
{
#if HAVE_BFD_H && HAVE_DEMANGLE_H
    bool success = false;
    char **matching;
    self_bfd = bfd_openr( processFullName().c_str(), NULL );
    if ( !bfd_check_format( self_bfd, bfd_archive) &&
            bfd_check_format_matches( self_bfd, bfd_object, &matching ) ) {

        if ( bfd_get_file_flags( self_bfd ) & HAS_SYMS ) {
            bfd_boolean dynamic = false;

            long storage = bfd_get_symtab_upper_bound( self_bfd );
            if ( !storage ) {
                storage = bfd_get_dynamic_symtab_upper_bound( self_bfd );
                dynamic = true;
            }
            if ( storage >= 0 ) {
                long symcnt;
                self_symbols = (asymbol **)malloc( storage );
                if (dynamic)
                    symcnt = bfd_canonicalize_dynamic_symtab( self_bfd, self_symbols );
                else
                    symcnt = bfd_canonicalize_symtab( self_bfd, self_symbols );
                if ( symcnt >= 0 ) {
                    success = true;
                } else {
                    free( self_symbols );
                    self_symbols = NULL;
                }
            }
        }
    }
    if ( !success && self_bfd ) {
        bfd_close( self_bfd );
        self_bfd = NULL;
        fprintf( stderr, "bfd setup failure\n" );
    }
#endif
}

static void cleanupSymbolTable()
{
#if HAVE_BFD_H && HAVE_DEMANGLE_H
    if ( self_bfd ) {
        bfd_close( self_bfd );
        self_bfd = NULL;
        if ( self_symbols ) {
            free( self_symbols );
            self_symbols = NULL;
        }
    }
#endif
}

BacktraceGenerator::BacktraceGenerator()
{
    if ( !trace_ref_count++ ) {
        pthread_mutex_init( &trace_mutex, NULL );
        symbol_buffer = (char *)malloc( 4096 );
        symbol_buffer_length = 4096;
        setupSymbolTable();
    }
}

BacktraceGenerator::~BacktraceGenerator()
{
    if ( ! --trace_ref_count ) {
        pthread_mutex_destroy( &trace_mutex );
        free( symbol_buffer );
        symbol_buffer = NULL;
        cleanupSymbolTable();
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

