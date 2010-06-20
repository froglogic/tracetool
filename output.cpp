#include "output.h"

using namespace Tracelib;
using namespace std;

void StdoutOutput::write( const vector<char> &data )
{
    fprintf(stdout, "%s\n", &data[0]);
}

