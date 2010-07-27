#ifndef TRACELIB_SERIALIZER_H
#define TRACELIB_SERIALIZER_H

#include "tracelib_config.h"

#include <string>
#include <vector>

TRACELIB_NAMESPACE_BEGIN

struct TraceEntry;
class VariableValue;

class Serializer
{
public:
    virtual ~Serializer();

    virtual std::vector<char> serialize( const TraceEntry &entry ) = 0;

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

private:
    std::string convertVariableValue( const VariableValue &v ) const;

    bool m_showTimestamp;
};

class CSVSerializer : public Serializer
{
public:
    virtual std::vector<char> serialize( const TraceEntry &entry );

private:
    std::string escape( const std::string &s ) const;
    std::string convertVariableValue( const VariableValue &v ) const;
};

class XMLSerializer : public Serializer
{
public:
    XMLSerializer();

    void setBeautifiedOutput( bool beautifiedOutput );

    virtual std::vector<char> serialize( const TraceEntry &entry );

private:
    std::string convertVariable( const char *name, const VariableValue &v ) const;

    bool m_beautifiedOutput;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_SERIALIZER_H)

