/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "configuration.h"
#include "errorlog.h"
#include "filter.h"
#include "output.h"
#include "serializer.h"
#include "trace.h"

#include "3rdparty/tinyxml/tinyxml.h"

#include <fstream>

#include <string.h>

using namespace std;

static bool fileExists( const string &filename )
{
   return ifstream( filename.c_str() ).is_open();
}

TRACELIB_NAMESPACE_BEGIN

Configuration *Configuration::fromFile( const string &fileName, ErrorLog *errorLog )
{
    Configuration *cfg = new Configuration( errorLog );
    if ( !cfg->loadFromFile( fileName ) ) {
        delete cfg;
        return 0;
    }
    return cfg;
}

Configuration *Configuration::fromMarkup( const string &markup, ErrorLog *errorLog )
{
    Configuration *cfg = new Configuration( errorLog );
    if ( !cfg->loadFromMarkup( markup ) ) {
        delete cfg;
        return 0;
    }
    return cfg;
}

Configuration::Configuration( ErrorLog *errorLog )
    : m_fileName( "<null>"),
    m_configuredSerializer( 0 ),
    m_configuredOutput( 0 ),
    m_errorLog( errorLog )
{
}

bool Configuration::loadFromFile( const string &fileName )
{
    m_fileName = fileName;;

    if ( !fileExists( m_fileName ) ) {
        return false;
    }

    TiXmlDocument xmlDoc;
    if ( !xmlDoc.LoadFile( m_fileName.c_str() ) ) {
        m_errorLog->write( "Tracelib Configuration: Failed to load XML file from %s", m_fileName.c_str() );
        return false;
    }

    return loadFrom( &xmlDoc );
}

bool Configuration::loadFromMarkup( const string &markup )
{
    TiXmlDocument xmlDoc;
    xmlDoc.Parse( markup.c_str() ); // XXX Error handling
    return loadFrom( &xmlDoc );
}

bool Configuration::loadFrom( TiXmlDocument *xmlDoc )
{
    TiXmlElement *rootElement = xmlDoc->RootElement();
    if ( rootElement->ValueStr() != "tracelibConfiguration" ) {
        m_errorLog->write( "Tracelib Configuration: while reading %s: unexpected root element '%s' found", m_fileName.c_str(), rootElement->Value() );
        return false;
    }

    static string myProcessName = currentProcessName();

    for ( TiXmlElement *e = rootElement->FirstChildElement(); e; e = e->NextSiblingElement() ) {
        if ( e->ValueStr() == "process" ) {
            TiXmlElement *nameElement = e->FirstChildElement( "name" );
            if ( !nameElement ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: found <process> element without <name> child element.", m_fileName.c_str() );
                return false;
            }


            // XXX Consider encoding issues (e.g. if myProcessName contains umlauts)
#ifdef _WIN32
            const bool isMyProcessElement = _stricmp( myProcessName.c_str(), nameElement->GetText() ) == 0;
#else
            const bool isMyProcessElement = strcmp( myProcessName.c_str(), nameElement->GetText() ) == 0;
#endif
            if ( isMyProcessElement ) {
                m_errorLog->write( "Tracelib Configuration: found configuration for process %s", myProcessName.c_str() );
                return readProcessElement( e );
            }
            continue;
        }

        m_errorLog->write( "Tracelib Configuration: while reading %s: unexpected child element '%s' found inside <tracelibConfiguration>.", m_fileName.c_str(), e->Value() );
        return false;
    }
    m_errorLog->write( "Tracelib Configuration: no configuration found for process %s", myProcessName.c_str() );
    return true;
}

bool Configuration::readProcessElement( TiXmlElement *processElement )
{
    for ( TiXmlElement *e = processElement->FirstChildElement(); e; e = e->NextSiblingElement() ) {
        if ( e->ValueStr() == "name" ) {
            continue;
        }

        if ( e->ValueStr() == "serializer" ) {
            if ( m_configuredSerializer ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: found multiple <serializer> elements in <process> element.", m_fileName.c_str() );
                return false;
            }

            Serializer *s = createSerializerFromElement( e );
            if ( !s ) {
                return false;
            }

            m_configuredSerializer = s;
            continue;
        }

        if ( e->ValueStr() == "tracepointset" ) {
            TracePointSet *tracePointSet = createTracePointSetFromElement( e );
            if ( !tracePointSet ) {
                return false;
            }
            m_configuredTracePointSets.push_back( tracePointSet );
            continue;
        }

        if ( e->ValueStr() == "output" ) {
            if ( m_configuredOutput ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: found multiple <output> elements in <process> element.", m_fileName.c_str() );
                return false;
            }
            Output *output = createOutputFromElement( e );
            if ( !output ) {
                return false;
            }
            m_configuredOutput = output;
            continue;
        }

        m_errorLog->write( "Tracelib Configuration: while reading %s: unexpected child element '%s' found inside <process>.", m_fileName.c_str(), processElement->Value() );
    }
    return true;
}

const vector<TracePointSet *> &Configuration::configuredTracePointSets() const
{
    return m_configuredTracePointSets;
}

Serializer *Configuration::configuredSerializer()
{
    return m_configuredSerializer;
}

Output *Configuration::configuredOutput()
{
    return m_configuredOutput;
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
        MatchingMode matchingMode;
        string matchingModeValue = "strict";
        if ( e->QueryStringAttribute( "matchingmode", &matchingModeValue ) == TIXML_SUCCESS ) {
            if ( matchingModeValue == "strict" ) {
                matchingMode = StrictMatch;
            } else if ( matchingModeValue == "regexp" ) {
                matchingMode = RegExpMatch;
            } else if ( matchingModeValue == "wildcard" ) {
                matchingMode = WildcardMatch;
            } else {
                m_errorLog->write( "Tracelib Configuration: while reading %s: unsupported matching mode '%s' specified for <pathfilter> element.", m_fileName.c_str(), matchingModeValue.c_str() );
                return 0;
            }
        }
        const char *pathFilterValue = e->GetText();
        if ( !pathFilterValue ) return 0;
        PathFilter *f = new PathFilter;
        f->setPath( matchingMode, pathFilterValue ); // XXX Consider encoding issues
        return f;
    }

    if ( e->ValueStr() == "functionfilter" ) {
        MatchingMode matchingMode;
        string matchingModeValue = "strict";
        if ( e->QueryStringAttribute( "matchingmode", &matchingModeValue ) == TIXML_SUCCESS ) {
            if ( matchingModeValue == "strict" ) {
                matchingMode = StrictMatch;
            } else if ( matchingModeValue == "regexp" ) {
                matchingMode = RegExpMatch;
            } else if ( matchingModeValue == "wildcard" ) {
                matchingMode = WildcardMatch;
            } else {
                m_errorLog->write( "Tracelib Configuration: while reading %s: unsupported matching mode '%s' specified for <functionfilter> element.", m_fileName.c_str(), matchingModeValue.c_str() );
                return 0;
            }
        }
        const char *functionFilterValue = e->GetText();
        if ( !functionFilterValue ) return 0;
        FunctionFilter *f = new FunctionFilter;
        f->setFunction( matchingMode, functionFilterValue ); // XXX Consider encoding issues
        return f;
    }

    if ( e->ValueStr() == "tracekeyfilter" ) {
        GroupFilter::Mode mode = GroupFilter::Whitelist;

        string modeValue;
        if ( e->QueryStringAttribute( "mode", &modeValue ) == TIXML_SUCCESS ) {
            if ( modeValue == "whitelist" ) {
                mode = GroupFilter::Whitelist;
            } else if ( modeValue == "blacklist" ) {
                mode = GroupFilter::Blacklist;
            } else {
                m_errorLog->write( "Tracelib Configuration: while reading %s: unsupported mode '%s' specified for <tracekeyfilter> element.", m_fileName.c_str(), modeValue.c_str() );
                return 0;
            }
        }

        GroupFilter *f = new GroupFilter;
        f->setMode( mode );
        for ( TiXmlElement *childElement = e->FirstChildElement(); childElement; childElement = childElement->NextSiblingElement() ) {
            if ( childElement->ValueStr() == "key" ) {
                f->addGroupName( childElement->GetText() ); // XXX Consider encoding issues
            } else {
                delete f;
                m_errorLog->write( "Tracelib Configuration: while reading %s: unsupported child element '%s' specified for <tracekeyfilter> element.", m_fileName.c_str(), childElement->ValueStr().c_str() );
                return 0;
            }
        }

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
        m_errorLog->write( "Tracelib Configuration: using plaintext serializer" );
        return serializer;
    }

    if ( serializerType == "xml" ) {
        bool beautifiedOutput = false;
        for ( TiXmlElement *optionElement = e->FirstChildElement(); optionElement; optionElement = optionElement->NextSiblingElement() ) {
            if ( optionElement->ValueStr() != "option" ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Unexpected element '%s' in <serializer> element of type xml found.", m_fileName.c_str(), optionElement->Value() );
                return 0;
            }

            string optionName;
            if ( optionElement->QueryStringAttribute( "name", &optionName ) != TIXML_SUCCESS ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Failed to read name property of <serializer> element; ignoring this.", m_fileName.c_str() );
                continue;
            }

            if ( optionName == "beautifiedOutput" ) {
                const char *beautifiedOutputValue = optionElement->GetText();
                if ( beautifiedOutputValue )
                    beautifiedOutput = strcmp( beautifiedOutputValue, "yes" ) == 0;
            } else {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Unknown <option> element with name '%s' found in xml serializer; ignoring this.", m_fileName.c_str(), optionName.c_str() );
                continue;
            }
        }
        XMLSerializer *serializer = new XMLSerializer;
        serializer->setBeautifiedOutput( beautifiedOutput );
        m_errorLog->write( "Tracelib Configuration: using XML serializer (beautified output=%d)", beautifiedOutput );
        return serializer;
    }

    m_errorLog->write( "Tracelib Configuration: while reading %s: <serializer> element with unknown type '%s' found.", m_fileName.c_str(), serializerType.c_str() );
    return 0;
}

TracePointSet *Configuration::createTracePointSetFromElement( TiXmlElement *e )
{
    string backtracesAttr = "no";
    e->QueryStringAttribute( "backtraces", &backtracesAttr );
    if ( backtracesAttr != "yes" && backtracesAttr != "no" ) {
        m_errorLog->write( "Tracelib Configuration: while reading %s: Invalid value '%s' for backtraces= attribute of <tracepointset> element", m_fileName.c_str(), backtracesAttr.c_str() );
        return 0;
    }

    string variablesAttr = "no";
    e->QueryStringAttribute( "variables", &variablesAttr );
    if ( variablesAttr != "yes" && variablesAttr != "no" ) {
        m_errorLog->write( "Tracelib Configuration: while reading %s: Invalid value '%s' for variables= attribute of <tracepointset> element", m_fileName.c_str(), variablesAttr.c_str() );
        return 0;
    }

    TiXmlElement *filterElement = e->FirstChildElement();
    if ( !filterElement ) {
        m_errorLog->write( "Tracelib Configuration: while reading %s: No filter element specified for <tracepointset> element", m_fileName.c_str() );
        return 0;
    }

    ConjunctionFilter *filter = new ConjunctionFilter;
    while ( filterElement ) {
        Filter *subFilter = createFilterFromElement( filterElement );
        if ( !subFilter ) {
            delete filter;
            return 0;
        }
        filter->addFilter( subFilter );
        filterElement = filterElement->NextSiblingElement();
    }

    int actions = TracePointSet::LogTracePoint;
    if ( backtracesAttr == "yes" ) {
        actions |= TracePointSet::YieldBacktrace;
    }
    if ( variablesAttr == "yes" ) {
        actions |= TracePointSet::YieldVariables;
    }

    return new TracePointSet( filter, actions );
}

Output *Configuration::createOutputFromElement( TiXmlElement *e )
{
    string outputType;
    if ( e->QueryStringAttribute( "type", &outputType ) == TIXML_NO_ATTRIBUTE ) {
        m_errorLog->write( "Tracelib Configuration: while reading %s: No type= attribute specified for <output> element", m_fileName.c_str() );
        return 0;
    }

    if ( outputType == "stdout" ) {
        m_errorLog->write( "Tracelib Configuration: using stdout output" );
        return new StdoutOutput;
    }

    if ( outputType == "tcp" ) {
        string hostname;
        unsigned short port = TRACELIB_DEFAULT_PORT;
        for ( TiXmlElement *optionElement = e->FirstChildElement(); optionElement; optionElement = optionElement->NextSiblingElement() ) {
            if ( optionElement->ValueStr() != "option" ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Unexpected element '%s' in <output> element of type tcp found.", m_fileName.c_str(), optionElement->Value() );
                return 0;
            }

            string optionName;
            if ( optionElement->QueryStringAttribute( "name", &optionName ) != TIXML_SUCCESS ) {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Failed to read name property of <option> element; ignoring this.", m_fileName.c_str() );
                continue;
            }

            if ( optionName == "host" ) {
                const char *hostValue = optionElement->GetText();
                if ( hostValue )
                    hostname = hostValue; // XXX Consider encoding issues
            } else if ( optionName == "port" ) {
                const char *portValue = optionElement->GetText();
                if ( portValue ) {
                    istringstream str( portValue );
                    str >> port; // XXX Error handling for non-numeric port numbers
               }
            } else {
                m_errorLog->write( "Tracelib Configuration: while reading %s: Unknown <option> element with name '%s' found in tcp output; ignoring this.", m_fileName.c_str(), optionName.c_str() );
                continue;
            }
        }

        if ( hostname.empty() ) {
            m_errorLog->write( "Tracelib Configuration: while reading %s: No 'host' option specified for <output> element of type tcp.", m_fileName.c_str() );
            return 0;
        }

        if ( port == 0 ) {
            m_errorLog->write( "Tracelib Configuration: while reading %s: No 'port' option specified for <output> element of type tcp.", m_fileName.c_str() );
            return 0;
        }

        m_errorLog->write( "Tracelib Configuration: using TCP/IP output, remote = %s:%d", hostname.c_str(), port );
        return new NetworkOutput( m_errorLog, hostname.c_str(), port );
    }

    m_errorLog->write( "Tracelib Configuration: while reading %s: Unknown type '%s' specified for <output> element", m_fileName.c_str(), outputType.c_str() );
    return 0;
}

TRACELIB_NAMESPACE_END

