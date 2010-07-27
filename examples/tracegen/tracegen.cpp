#include "tracelib.h"
#include "serializer.h"
#include "filter.h"
#include "output.h"
#include <windows.h>
#include <time.h>
#include <sstream>

TRACELIB_NAMESPACE_BEGIN
    template <>
    VariableValue convertVariable<time_t>( time_t value ) {
        std::ostringstream str;
        str << value;
        return VariableValue::stringValue( str.str() );
    }
TRACELIB_NAMESPACE_END

static VOID CALLBACK timerProc( HWND, UINT, UINT_PTR, DWORD )
{
    TRACELIB_WATCH(TRACELIB_VAR(time(NULL)))
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
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

    return (int)msg.wParam;
}
