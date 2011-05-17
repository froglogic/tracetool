/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_SERIALIZER_H
#define TRACELIB_SERIALIZER_H

#include "tracelib_config.h"

#include <string>
#include <vector>

#include "configuration.h" // for StorageConfiguration

TRACELIB_NAMESPACE_BEGIN

struct TraceEntry;
struct ProcessShutdownEvent;
class VariableValue;

class Serializer
{
public:
    virtual ~Serializer();

    virtual std::vector<char> serialize( const TraceEntry &entry ) = 0;
    virtual std::vector<char> serialize( const ProcessShutdownEvent &ev ) = 0;

    virtual void setStorageConfiguration( const StorageConfiguration &cfg ) { }

protected:
    Serializer();

private:
    Serializer( const Serializer &rhs );
    void operator=( const Serializer &other );
};

class PlaintextSerializer : public Serializer
{
public:
    PlaintextSerializer();

    void setTimestampsShown( bool timestamps );
    virtual std::vector<char> serialize( const TraceEntry &entry );
    virtual std::vector<char> serialize( const ProcessShutdownEvent &ev );

private:
    std::string convertVariableValue( const VariableValue &v ) const;

    bool m_showTimestamp;
};

class XMLSerializer : public Serializer
{
public:
    XMLSerializer();

    void setBeautifiedOutput( bool beautifiedOutput );

    virtual std::vector<char> serialize( const TraceEntry &entry );
    virtual std::vector<char> serialize( const ProcessShutdownEvent &ev );

    virtual void setStorageConfiguration( const StorageConfiguration &cfg ) {
        m_cfg = cfg;
    }

private:
    std::string convertVariable( const char *name, const VariableValue &v ) const;

    bool m_beautifiedOutput;
    StorageConfiguration m_cfg;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_SERIALIZER_H)

