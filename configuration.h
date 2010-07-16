#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

class TiXmlElement;

namespace Tracelib
{

class Filter;
class Serializer;

class Configuration
{
public:
    typedef void(*ErrorFunction)(const std::string &msg);

    Configuration( ErrorFunction errorFn = 0 );

    Filter *configuredFilter();
    Serializer *configuredSerializer();

private:
    static std::string currentProcessName();
    static std::string configurationFileName();

    Filter *createFilterFromElement( TiXmlElement *e );
    Serializer *createSerializerFromElement( TiXmlElement *e );

    Filter *m_configuredFilter;
    Serializer *m_configuredSerializer;
    ErrorFunction m_errorFn;
};

}

#endif // !defined(CONFIGURATION_H)
