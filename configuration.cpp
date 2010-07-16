#include "configuration.h"
#include "errorlog.h"
#include "filter.h"
#include "serializer.h"

#include "3rdparty/tinyxml/tinyxml.h"

#include <fstream>

#include <string.h>

using namespace Tracelib;
using namespace std;

static bool fileExists( const string &filename )
{
   return ifstream( filename.c_str() ).is_open();
}

Configuration::Configuration()
    : m_configuredFilter( 0 ),
    m_configuredSerializer( 0 ),
    m_errorLog( new DebugViewErrorLog ),
    m_fileName( configurationFileName() )
{
    if ( !fileExists( m_fileName ) ) {
        return;
    }

    TiXmlDocument xmlDoc;
    if ( !xmlDoc.LoadFile( m_fileName.c_str() ) ) {
        m_errorLog->write( "Tracelib Configuration: Failed to load XML file from %s", m_fileName.c_str() );
        return;;
    }

    TiXmlElement *rootElement = xmlDoc.RootElement();
    if ( rootElement->ValueStr() != "tracelibConfiguration" ) {
        m_errorLog->write( "Tracelib Configuration: while reading %s: unexpected root element '%s' found", m_fileName.c_str(), rootElement->Value() );
        return;
    }

    TiXmlElement *processElement = rootElement->FirstChildElement();
    while ( processElement ) {
        if ( processElement->ValueStr() != "process" ) {
            m_errorLog->write( "Tracelib Configuration: while reading %s: unexpected child element '%s' found inside <tracelibConfiguration>.", m_fileName.c_str(), processElement->Value() );
            return;
        }

        TiXmlElement *nameElement = processElement->FirstChildElement( "name" );
        if ( !nameElement ) {
            m_errorLog->write( "Tracelib Configuration: while reading %s: found <process> element without <name> child element.", m_fileName.c_str() );
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
                        m_errorLog->write( "Tracelib Configuration: while reading %s: found multiple <serializer> elements in <process> element.", m_fileName.c_str() );
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
            m_errorLog->write( "Tracelib Configuration: while reading %s: <verbosityfilter> element requires maxVerbosity attribute to be an integer value.", m_fileName.c_str() );
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
            m_errorLog->write( "Tracelib Configuration: while reading %s: <pathfilter> element requires fromLine attribute to be an integer value.", m_fileName.c_str() );
            return 0;
        }

        int toLine;
        rc = e->QueryIntAttribute( "toLine", &toLine );
        if ( rc != TIXML_SUCCESS && rc != TIXML_NO_ATTRIBUTE ) {
            m_errorLog->write( "Tracelib Configuration: while reading %s: <pathfilter> element requires toLine attribute to be an integer value.", m_fileName.c_str() );
            return 0;
        }

        PathFilter *f = new PathFilter;
        f->setPath( Tracelib::StrictMatch, e->GetText() );
        return f;
    }

    if ( e->ValueStr() == "functionfilter" ) {
        FunctionFilter *f = new FunctionFilter;
        f->setFunction( Tracelib::StrictMatch, e->GetText() );
        return f;
    }

    m_errorLog->write( "Tracelib Configuration: while reading %s: Unexpected filter element '%s' found.", m_fileName.c_str(), e->Value() );
    return 0;
}

Serializer *Configuration::createSerializerFromElement( TiXmlElement *e )
{
    string serializerType;
    if ( e->QueryStringAttribute( "type", &serializerType ) != TIXML_SUCCESS ) {
        m_errorLog->write( "Tracelib Configuration: while reading %s: Failed to read type property of <serializer> element.", m_fileName.c_str() );
        return 0;
    }

    if ( serializerType == "plaintext" ) {
        PlaintextSerializer *serializer = new PlaintextSerializer;
        for ( TiXmlElement *optionElement = e->FirstChildElement(); optionElement; optionElement = optionElement->NextSiblingElement() ) {
            if ( optionElement->ValueStr() != "option" ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Unexpected element '%s' in <serializer> element of type plaintext found.", m_fileName.c_str(), optionElement->Value() );
                delete serializer;
                return 0;
            }

            string optionName;
            if ( optionElement->QueryStringAttribute( "name", &optionName ) != TIXML_SUCCESS ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Failed to read name property of <option> element; ignoring this.", m_fileName.c_str() );
                continue;
            }

            if ( optionName == "timestamps" ) {
                serializer->setTimestampsShown( strcmp( optionElement->GetText(), "yes" ) == 0 );
            } else {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Unknown <option> element with name '%s' found in plaintext serializer; ignoring this.", m_fileName.c_str(), optionName.c_str() );
                continue;
            }
        }
        return serializer;
    }

    if ( serializerType == "csv" ) {
        return new CSVSerializer;
    }

    m_errorLog->write( "Tracelib Configuration: while reading %s: <serializer> element with unknown type '%s' found.", m_fileName.c_str(), serializerType.c_str() );
    return 0;
}

