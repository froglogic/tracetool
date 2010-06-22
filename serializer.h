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
    virtual std::vector<char> serialize( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName, const std::vector<AbstractVariableConverter *> &variables );

private:
    bool m_showTimestamp;
};

class CSVSerializer : public Serializer
{
public:
    virtual std::vector<char> serialize( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName, const std::vector<AbstractVariableConverter *> &variables );

private:
    std::string escape( const std::string &s ) const;
};

}

#endif // !defined(SERIALIZER_H)

