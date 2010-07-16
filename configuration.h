#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

namespace Tracelib
{

class Filter;

class Configuration
{
public:
    Configuration();

    Filter *configuredFilter();

private:
    static std::string currentProcessName();
    static std::string configurationFileName();

    Filter *m_configuredFilter;
};

}

#endif // !defined(CONFIGURATION_H)
