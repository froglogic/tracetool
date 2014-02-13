#include <iostream>
#include <string>

#include "tracelib.h"

int main()
{
    fTrace(0) << "main() entered";

    using namespace std;

    string name;
    cout << "Please enter your name: ";
    cin >> name;

    fWatch(0) << fVar(name);

    cout << "Hello, " << name << "!" << endl;

    fTrace(0) << "main() finished";
}

