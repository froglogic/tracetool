#include "tracelib.h"
#include "serializer.h"
#include "filter.h"
#include "output.h"
#include <windows.h>
#include <time.h>
#include <sstream>

namespace Tracelib {
    template <>
    std::string convertVariable<time_t>( time_t value ) {
        std::ostringstream str;
        str << value;
        return str.str();
    }
}

int main()
{
    using namespace Tracelib;
    Trace *trace = new Trace;
    trace->setSerializer( new CSVSerializer );
    trace->setOutput( new StdoutOutput );
    trace->addFilter( new VerbosityFilter );
    setActiveTrace( trace );
    while ( true ) {
        TRACELIB_SNAPSHOT(1) << TRACELIB_VAR(time(NULL));
        ::Sleep(1000);
    }
}
