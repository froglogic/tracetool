#include "configuration.h"
#include "filter.h"
#include "winstringconv.h"

#include "3rdparty/simpleini/SimpleIni.h"

#include <sstream>

using namespace Tracelib;
using namespace std;

static wstring configurationFileName()
{
    /* According to the MSDN documentation on GetEnvironmentVariable, an
     * environment variable has a maximum size limit of 32767 characters
     * including the terminating null.
     */
    wchar_t buf[ 32767 ] = { L'0' };
    if ( ::GetEnvironmentVariableW( L"TRACELIB_CONFIG_FILE",
                                    buf, sizeof( buf ) / sizeof( buf[0] ) ) ) {
            return &buf[0];
    }

    ::GetModuleFileNameW( NULL, buf, sizeof( buf ) / sizeof( buf[0] ) );
    wchar_t *lastSeparator = wcsrchr( buf, L'\\' );
    wcscpy( lastSeparator + 1, L"tracelib.ini" ); // XXX Guard against buffer overflow!
    return &buf[0];
}

static wstring executableName()
{
    wchar_t buf[ 32768 ];
    ::GetModuleFileNameW( NULL, buf, sizeof( buf ) / sizeof( buf[0] ) );
    wchar_t *lastSeparator = wcsrchr( buf, L'\\' );
    return lastSeparator + 1;
}

static ConjunctionFilter *createConjunctionForSection( CSimpleIniW *config, const wchar_t *section )
{
    CSimpleIniW::TNamesDepend keys;
    if ( !config->GetAllKeys( section, keys ) ) {
        return 0;
    }

    ConjunctionFilter *filter = new ConjunctionFilter;
    CSimpleIniW::TNamesDepend::const_iterator it, end = keys.end();
    for ( it = keys.begin(); it != end; ++it ) {
        if ( wcscmp( it->pItem, L"Verbosity" ) == 0 ) {
            const wchar_t *value = config->GetValue( section, it->pItem );
            wistringstream stream( value );
            int verbosity;
            stream >> verbosity; // XXX Consider non-integer values of 'value'
            VerbosityFilter *subFilter = new VerbosityFilter;
            subFilter->setMaximumVerbosity( verbosity );
            filter->addFilter( subFilter );
            continue;
        }
        if ( wcscmp( it->pItem, L"Path" ) == 0 ) {
            const wchar_t *value = config->GetValue( section, it->pItem );
            SourceFilePathFilter *subFilter = new SourceFilePathFilter;
            subFilter->setPath( Squish::utf16ToUTF8( value ) ); // XXX encoding right?
            filter->addFilter( subFilter );
            continue;
        }
    }
    return filter;
}

static DisjunctionFilter *createDisjunctionForSection( CSimpleIniW *config, const wchar_t *section )
{
    CSimpleIniW::TNamesDepend includedSets;
    if ( !config->GetAllValues( section, L"IncludeSet", includedSets ) ) {
        return 0;
    }

    DisjunctionFilter *filter = new DisjunctionFilter;
    CSimpleIniW::TNamesDepend::const_iterator it, end = includedSets.end();
    for ( it = includedSets.begin(); it != end; ++it ) {
        ConjunctionFilter *conjunction = createConjunctionForSection( config, it->pItem );
        if ( conjunction ) {
            filter->addFilter( conjunction );
        }
    }

    return filter;
}

namespace Tracelib
{

void configureTrace( Trace *trace )
{
    const wstring configFileName = configurationFileName();
    if ( configFileName.empty() ) {
        return;
    }

    CSimpleIniW config;
    config.SetUnicode( true );
    if ( config.LoadFile( configFileName.c_str() ) < 0 ) {
        // XXX Yield diagnostic output.
        return;
    }

    const wstring exeName = executableName();

    CSimpleIniW::TNamesDepend sectionNames;
    config.GetAllSections( sectionNames );
    CSimpleIniW::TNamesDepend::const_iterator sectionIt, sectionEnd = sectionNames.end();
    for ( sectionIt = sectionNames.begin(); sectionIt != sectionEnd; ++sectionIt ) {
        if ( exeName == sectionIt->pItem ) {
            break;
        }
    }

    // No configuration group for the current process? Bail out.
    if ( sectionIt == sectionNames.end() ) {
        return;
    }


    trace->setFilter( createDisjunctionForSection( &config, sectionIt->pItem ) );
}

}

