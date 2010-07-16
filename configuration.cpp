#include "configuration.h"
#include "filter.h"
#include "serializer.h"

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
    m_configuredSerializer( 0 ),
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

                if ( e->ValueStr() == "serializer" ) {
                    if ( m_configuredSerializer ) {
                        m_errorFn( "Found multiple <serializer> elements in <process> element." );
                        return;
                    }
                    m_configuredSerializer = createSerializerFromElement( e );
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

Serializer *Configuration::configuredSerializer()
{
    return m_configuredSerializer;
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

Serializer *Configuration::createSerializerFromElement( TiXmlElement *e )
{
    string serializerType;
    if ( e->QueryStringAttribute( "type", &serializerType ) != TIXML_SUCCESS ) {
        m_errorFn( "Failed to read type property of <serializer> element." );
        return 0;
    }

    if ( serializerType == "plaintext" ) {
        PlaintextSerializer *serializer = new PlaintextSerializer;
        for ( TiXmlElement *optionElement = e->FirstChildElement(); optionElement; optionElement = optionElement->NextSiblingElement() ) {
            if ( optionElement->ValueStr() != "option" ) {
                m_errorFn( "Unexpected element in <serializer> of type plaintext found." );
                delete serializer;
                return 0;
            }

            string optionName;
            if ( optionElement->QueryStringAttribute( "name", &optionName ) != TIXML_SUCCESS ) {
                m_errorFn( "Failed to read name property of <option> element; ignoring this." );
                continue;
            }

            if ( optionName == "timestamps" ) {
                serializer->setTimestampsShown( strcmp( optionElement->GetText(), "yes" ) == 0 );
            } else {
                m_errorFn( "Found <option> element with unknown name in plaintext serializer; ignoring this." );
                continue;
            }
        }
        return serializer;
    }

    if ( serializerType == "csv" ) {
        return new CSVSerializer;
    }

    m_errorFn( "Unknown serializer type found." );
    return 0;

}

