#ifndef TRACELIB_CONFIGURATION_H
#define TRACELIB_CONFIGURATION_H

#include "tracelib_config.h"

#include <string>
#include <vector>

class TiXmlDocument;
class TiXmlElement;

TRACELIB_NAMESPACE_BEGIN

class ErrorLog;
class Filter;
class Output;
class Serializer;
class TracePointSet;

class Configuration
{
public:
    static std::string defaultFileName();
    static std::string currentProcessName();

    static Configuration *fromFile( const std::string &fileName );
    static Configuration *fromMarkup( const std::string &markup );

    const std::vector<TracePointSet *> &configuredTracePointSets() const;
    Serializer *configuredSerializer();
    Output *configuredOutput();

private:
    Configuration();
    bool loadFromFile( const std::string &fileName );
    bool loadFromMarkup( const std::string &markup );
    bool loadFrom( TiXmlDocument *xmlDoc );

    Filter *createFilterFromElement( TiXmlElement *e );
    Serializer *createSerializerFromElement( TiXmlElement *e );
    TracePointSet *createTracePointSetFromElement( TiXmlElement *e );
    Output *createOutputFromElement( TiXmlElement *e );

    std::string m_fileName;
    std::vector<TracePointSet *> m_configuredTracePointSets;
    Serializer *m_configuredSerializer;
    Output *m_configuredOutput;
    ErrorLog *m_errorLog;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_CONFIGURATION_H)
