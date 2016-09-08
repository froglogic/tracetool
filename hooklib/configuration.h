/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRACELIB_CONFIGURATION_H
#define TRACELIB_CONFIGURATION_H

#include "tracelib_config.h"

#include <string>
#include <vector>

class TiXmlDocument;
class TiXmlElement;

TRACELIB_NAMESPACE_BEGIN

class Log;
class Filter;
class Output;
class Serializer;
class TracePointSet;

struct StorageConfiguration {
    static const unsigned long UnlimitedTraceSize = 0;

    StorageConfiguration()
        : maximumTraceSize( UnlimitedTraceSize ),
          shrinkPercentage( 10 )
    { }

    unsigned long maximumTraceSize;
    unsigned short shrinkPercentage;
    std::string archiveDirectoryName;
};

struct TraceKey
{
    TraceKey() : enabled( true ) { }
    bool enabled;
    std::string name;
};

class Configuration
{
public:
    static std::string defaultFileName();
    static std::string currentProcessName();
    static std::string pathSeparator();
    static std::string userHome();
    static bool isAbsolute( const std::string &filename );
    static std::string executableName( const std::string &basename );

    static Configuration *fromFile( const std::string &fileName, Log *log );
    static Configuration *fromMarkup( const std::string &markup, Log *log );

    const StorageConfiguration &storageConfiguration() const;
    const std::vector<TracePointSet *> &configuredTracePointSets() const;
    Serializer *configuredSerializer();
    Output *configuredOutput();
    const std::vector<TraceKey> &configuredTraceKeys() const;

private:
    explicit Configuration( Log *log );
    bool loadFromFile( const std::string &fileName );
    bool loadFromMarkup( const std::string &markup );
    bool loadFrom( TiXmlDocument *xmlDoc );

    Filter *createFilterFromElement( TiXmlElement *e );
    Serializer *createSerializerFromElement( TiXmlElement *e );
    TracePointSet *createTracePointSetFromElement( TiXmlElement *e );
    Output *createOutputFromElement( TiXmlElement *e );

    bool readProcessElement( TiXmlElement *e );
    bool readTraceKeysElement( TiXmlElement *e );
    bool readStorageElement( TiXmlElement *e );

    std::string m_fileName;
    std::vector<TracePointSet *> m_configuredTracePointSets;
    Serializer *m_configuredSerializer;
    Output *m_configuredOutput;
    Log *m_log;
    std::vector<TraceKey> m_configuredTraceKeys;
    StorageConfiguration m_storageConfiguration;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_CONFIGURATION_H)
