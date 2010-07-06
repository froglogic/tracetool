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

static VOID CALLBACK timerProc( HWND, UINT, UINT_PTR, DWORD )
{
    TRACELIB_SNAPSHOT(1) << TRACELIB_VAR(time(NULL));
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
    using namespace Tracelib;
    Trace *trace = new Trace;
    trace->setSerializer( new CSVSerializer );
    MultiplexingOutput *output = new MultiplexingOutput;
    output->addOutput( new StdoutOutput );
    output->addOutput( new NetworkOutput( "127.0.0.1", 44123 ) );
    trace->setOutput( output );
    trace->setFilter( new VerbosityFilter );
    setActiveTrace( trace );

    SetTimer( NULL, 0, 1000, &timerProc );

    MSG msg;
    BOOL ret;
    while ( ( ret = GetMessage( &msg, NULL, 0, 0 ) ) != 0 ) {
        if ( ret == -1 ) {
            OutputDebugStringA( "GetMessage failed" );
        } else {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    return msg.wParam;
}
