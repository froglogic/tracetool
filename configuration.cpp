#include "configuration.h"
#include "filter.h"

#include "3rdparty/tinyxml/tinyxml.h"

#include <fstream>

#include <string.h>

using namespace Tracelib;
using namespace std;

static void nullErrorFn( const string & ) { }

static bool fileExists( const string &filename )
{
   return ifstream( filename.c_str() ).is_open();
}

Configuration::Configuration( ErrorFunction errorFn )
    : m_configuredFilter( 0 ),
    m_errorFn( errorFn ? errorFn : &nullErrorFn )
{
    const string fileName = configurationFileName();

    if ( !fileExists( fileName ) ) {
        return;
    }

    TiXmlDocument xmlDoc;
    if ( !xmlDoc.LoadFile( fileName.c_str() ) ) {
        m_errorFn( "Failed to load XML file." );
        return;;
    }

    TiXmlElement *rootElement = xmlDoc.RootElement();
    if ( rootElement->ValueStr() != "tracelibConfiguration" ) {
        m_errorFn( "Root element of XML file is not tracelibConfiguration." );
        return;
    }

    TiXmlElement *processElement = rootElement->FirstChildElement();
    while ( processElement ) {
        if ( processElement->ValueStr() != "process" ) {
            m_errorFn( "Unexpected child element found inside <tracelibConfiguration>." );
            return;
        }

        TiXmlElement *nameElement = processElement->FirstChildElement( "name" );
        if ( !nameElement ) {
            m_errorFn( "Found <process> element without <name> child element." );
            return;
        }


        static string myProcessName = currentProcessName();

#ifdef _WIN32
        const bool isMyProcessElement = _stricmp( myProcessName.c_str(), nameElement->GetText() ) == 0;
#else
        const bool isMyProcessElement = strcmp( myProcessName.c_str(), nameElement->GetText() ) == 0;
#endif
        if ( isMyProcessElement ) {
            for ( TiXmlElement *e = processElement->FirstChildElement(); e && !m_configuredFilter; e = e->NextSiblingElement() ) {
                if ( e->ValueStr() == "name" ) {
                    continue;
                }

                m_configuredFilter = createFilterFromElement( e );
            }
            break;
        }

        processElement = processElement->NextSiblingElement();
    }
}

Filter *Configuration::configuredFilter()
{
    return m_configuredFilter;
}

Filter *Configuration::createFilterFromElement( TiXmlElement *e )
{
    if ( e->ValueStr() == "matchanyfilter" ) {
        DisjunctionFilter *f = new DisjunctionFilter;
        for ( TiXmlElement *childElement = e->FirstChildElement(); childElement; childElement = childElement->NextSiblingElement() ) {
            Filter *subFilter = createFilterFromElement( childElement );
            if ( !subFilter ) {
                // XXX Yield diagnostics;
                delete f;
                return 0;
            }
            f->addFilter( subFilter );
        }
        return f;
    }
    if ( e->ValueStr() == "matchallfilter" ) {
        ConjunctionFilter *f = new ConjunctionFilter;
        for ( TiXmlElement *childElement = e->FirstChildElement(); childElement; childElement = childElement->NextSiblingElement() ) {
            Filter *subFilter = createFilterFromElement( childElement );
            if ( !subFilter ) {
                // XXX Yield diagnostics;
                delete f;
                return 0;
            }
            f->addFilter( subFilter );
        }
        return f;
    }
    if ( e->ValueStr() == "verbosityfilter" ) {
        int verbosity;
        if ( e->QueryIntAttribute( "maxVerbosity", &verbosity ) != TIXML_SUCCESS ) {
            m_errorFn( "<verbosityfilter> element requires maxVerbosity attribute to be an integer value." );
            return 0;
        }

        VerbosityFilter *f = new VerbosityFilter;
        f->setMaximumVerbosity( verbosity );
        return f;
    }
    if ( e->ValueStr() == "pathfilter" ) {
        int fromLine;
        int rc = e->QueryIntAttribute( "fromLine", &fromLine );
        if ( rc != TIXML_SUCCESS && rc != TIXML_NO_ATTRIBUTE ) {
            m_errorFn( "<pathfilter> element requires fromLine attribute to be an integer value." );
            return 0;
        }

        int toLine;
        rc = e->QueryIntAttribute( "toLine", &toLine );
        if ( rc != TIXML_SUCCESS && rc != TIXML_NO_ATTRIBUTE ) {
            m_errorFn( "<pathfilter> element requires tiLine attribute to be an integer value." );
            return 0;
        }

        PathFilter *f = new PathFilter;
        f->setPath( e->GetText() );
        return f;
    }
    if ( e->ValueStr() == "functionfilter" ) {
        // XXX Implement me
    }
    m_errorFn( "Unexpected element found" );
    return 0;
}

