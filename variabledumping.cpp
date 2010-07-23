#include "variabledumping.h"

TRACELIB_NAMESPACE_BEGIN

std::vector<AbstractVariable *> &operator<<( std::vector<AbstractVariable *> &v,
                                             AbstractVariable *c )
{
    v.push_back( c );
    return v;
}

TRACELIB_NAMESPACE_END
