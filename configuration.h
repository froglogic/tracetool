#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

class TiXmlElement;

namespace Tracelib
{

class ErrorLog;
class Filter;
class Serializer;

class Configuration
{
public:
    Configuration();

    Filter *configuredFilter();
    Serializer *configuredSerializer();

private:
    static std::string currentProcessName();
    static std::string configurationFileName();

    Filter *createFilterFromElement( TiXmlElement *e );
    Serializer *createSerializerFromElement( TiXmlElement *e );

    const std::string m_fileName;
    Filter *m_configuredFilter;
    Serializer *m_configuredSerializer;
    ErrorLog *m_errorLog;
};

}

#endif // !defined(CONFIGURATION_H)
