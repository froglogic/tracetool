#include "configuration.h"
#include "filter.h"

using namespace Tracelib;
using namespace std;

Configuration::Configuration()
    : m_configuredFilter( 0 )
{
}

Filter *Configuration::configuredFilter()
{
    return m_configuredFilter;
}

