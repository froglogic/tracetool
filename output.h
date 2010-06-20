#ifndef OUTPUT_H
#define OUTPUT_H

#include "tracelib.h"

namespace Tracelib
{

class TRACELIB_EXPORT StdoutOutput : public Output
{
public:
    virtual void write( const std::vector<char> &data );
};

}

#endif // !defined(OUTPUT_H)

