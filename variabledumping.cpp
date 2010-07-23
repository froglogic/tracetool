#include "variabledumping.h"

TRACELIB_NAMESPACE_BEGIN


VariableSnapshot &operator<<( VariableSnapshot &snapshot, AbstractVariable *v )
{
    snapshot.push_back( v );
    return snapshot;
}

TRACELIB_NAMESPACE_END
