#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
#include <vector>

class TiXmlElement;

namespace Tracelib
{

class ErrorLog;
class Filter;
class Serializer;
class TracePointSet;

class Configuration
{
public:
    Configuration();

    const std::vector<TracePointSet *> &configuredTracePointSets() const;
    Serializer *configuredSerializer();

private:
    static std::string currentProcessName();
    static std::string configurationFileName();

    Filter *createFilterFromElement( TiXmlElement *e );
    Serializer *createSerializerFromElement( TiXmlElement *e );
    TracePointSet *createTracePointSetFromElement( TiXmlElement *e );

    const std::string m_fileName;
    std::vector<TracePointSet *> m_configuredTracePointSets;
    Serializer *m_configuredSerializer;
    ErrorLog *m_errorLog;
};

}

#endif // !defined(CONFIGURATION_H)
