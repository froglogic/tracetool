#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "tracelib.h"

namespace Tracelib
{

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

}

#endif // !defined(SERIALIZER_H)

