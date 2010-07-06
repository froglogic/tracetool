#include "tracelib.h"
#include "filter.h"

#include "3rdparty/simpleini/SimpleIni.h"

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
    wcscpy( lastSeparator + 1, L"tracelib.ini" );
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
    return 0;
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

static void configureTrace( Trace *trace )
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

Output::Output()
{
}

Output::~Output()
{
}

Serializer::Serializer()
{
}

Serializer::~Serializer()
{
}

Filter::Filter()
{
}

Filter::~Filter()
{
}

SnapshotCreator::SnapshotCreator( Trace *trace, unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName )
    : m_trace( trace ),
    m_verbosity( verbosity ),
    m_sourceFile( sourceFile ),
    m_lineno( lineno ),
    m_functionName( functionName )
{
}

SnapshotCreator::~SnapshotCreator()
{
    m_trace->addEntry( m_verbosity, m_sourceFile, m_lineno, m_functionName, m_variables );
    vector<AbstractVariableConverter *>::iterator it, end = m_variables.end();
    for ( it = m_variables.begin(); it != end; ++it ) {
        delete *it;
    }
}

SnapshotCreator &SnapshotCreator::operator<<( AbstractVariableConverter *converter )
{
    m_variables.push_back( converter );
    return *this;
}

Trace::Trace()
    : m_serializer( 0 ),
    m_output( 0 )
{
}

Trace::~Trace()
{
    delete m_serializer;
    delete m_output;
    delete m_filter;
}

void Trace::addEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName, const vector<AbstractVariableConverter *> &variables )
{
    configure();

    if ( !m_output->canWrite() ) {
        return;
    }

    if ( !m_filter->acceptsEntry( verbosity, sourceFile, lineno, functionName ) ) {
        return;
    }

    vector<char> data = m_serializer->serialize( verbosity, sourceFile, lineno, functionName, variables );
    if ( !data.empty() ) {
        m_output->write( data );
    }
}

void Trace::setSerializer( Serializer *serializer )
{
    delete m_serializer;
    m_serializer = serializer;
}

void Trace::setOutput( Output *output )
{
    delete m_output;
    m_output = output;
}

void Trace::setFilter( Filter *filter )
{
    delete m_filter;
    m_filter = filter;
}

void Trace::configure()
{
    static bool configured = false;
    if ( configured ) {
        return;
    }
    configured = true;

    configureTrace( this );
}

static Trace *g_activeTrace = 0;

namespace Tracelib
{

Trace *getActiveTrace()
{
    return g_activeTrace;
}

void setActiveTrace( Trace *trace )
{
    g_activeTrace = trace;
}

}

