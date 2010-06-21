#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "tracelib.h"

namespace Tracelib
{

class TRACELIB_EXPORT PlaintextSerializer : public Serializer
{
public:
    PlaintextSerializer();

    void setTimestampsShown( bool timestamps );
    virtual std::vector<char> serialize( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName, const std::vector<AbstractVariableConverter *> &variables );

private:
    bool m_showTimestamp;
};

}

#endif // !defined(SERIALIZER_H)

