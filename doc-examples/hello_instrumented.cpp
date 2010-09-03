#include <iostream>
#include <string>

#include "tracelib.h"

int main()
{
    TRACELIB_TRACE_MSG( "main() entered" );

    using namespace std;

    string name;
    cout << "Please enter your name: ";
    cin >> name;

    TRACELIB_WATCH(TRACELIB_VAR(name));

    cout << "Hello, " << name << "!" << endl;

    TRACELIB_TRACE_MSG( "main() finished" );
}

