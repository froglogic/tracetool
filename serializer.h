#ifndef TRACELIB_SERIALIZER_H
#define TRACELIB_SERIALIZER_H

#include "tracelib_config.h"
#include "tracelib.h"

TRACELIB_NAMESPACE_BEGIN

class PlaintextSerializer : public Serializer
{
public:
    PlaintextSerializer();

    void setTimestampsShown( bool timestamps );
    virtual std::vector<char> serialize( const TraceEntry &entry );

private:
    bool m_showTimestamp;
};

class CSVSerializer : public Serializer
{
public:
    virtual std::vector<char> serialize( const TraceEntry &entry );

private:
    std::string escape( const std::string &s ) const;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_SERIALIZER_H)

