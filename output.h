#ifndef OUTPUT_H
#define OUTPUT_H

#include "tracelib.h"

namespace Tracelib
{

class StdoutOutput : public Output
{
public:
    virtual void write( const std::vector<char> &data );
};

}

#endif // !defined(OUTPUT_H)

