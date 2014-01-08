/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "getcurrentthreadid.h"

#include <string>
#include <iostream>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <unistd.h>
#endif

using namespace std;

static void printHelp(const char *appName)
{
    cout << "Usage: " << appName << " [OPTION]" << endl
         << "Possible options are:" << endl
         << "\t--processid"
         << endl;
}

static void suggestHelp(const char *appName)
{
    cout << "Try '" << appName << " --help' for more information." << endl;
}

int g_failureCount = 0;
int g_verificationCount = 0;

// JUnit-style
template <typename T>
static void assertEquals(const char *message, T expected, T actual)
{
    if (expected == actual) {
        cout << "PASS: " << message << "; got expected '"
             << boolalpha << expected << "'" << endl;
    } else {
        cout << "FAIL: " << message << "; expected '"
             << boolalpha << expected << "', got '"
             << boolalpha << actual << "'" << endl;
        ++g_failureCount;
    }
    ++g_verificationCount;
}

static void assertTrue(const char *message, bool condition)
{
    assertEquals(message, true, condition);
}


TRACELIB_NAMESPACE_BEGIN

static void test_getCurrentProcessId()
{
    {
        ProcessId pid1 = getCurrentProcessId();
        ProcessId pid2 = getCurrentProcessId();
        assertTrue("Repeated call consistency", pid1 == pid2);
    }
    {
        ProcessId pid = getCurrentProcessId();
        assertTrue("Non-zero positive value", pid > 0);
    }
}

static void test_getCurrentThreadId()
{
   {
        ThreadId tid1 = getCurrentThreadId();
        ThreadId tid2 = getCurrentThreadId();
        assertTrue("Repeated call consistency", tid1 == tid2);
    }
    {
        ProcessId tid = getCurrentThreadId();
        assertTrue("Positive value", tid >= 0);
    }
}

static void sleepMilliSeconds(int msecs)
{
#ifdef _WIN32
    ::Sleep(msecs);
#else
    ::usleep(msecs * 1000);
#endif
}

static void test_getCurrentProcessStartTime()
{
    {
        sleepMilliSeconds(1000);
        time_t t0 = getCurrentProcessStartTime();
        sleepMilliSeconds(1000);
        time_t t1 = getCurrentProcessStartTime();
        assertTrue("Start Time should not change", t0 == t1);
    }
}

TRACELIB_NAMESPACE_END

int main(int argc, char **argv)
{
    const char *appName = argv[0];

    if (argc != 2) {
        cout << appName << ": Missing command line argument." << endl;
        suggestHelp(appName);
        return 1;
    }

    string arg1 = argv[1];
    if (arg1 == "--help") {
        printHelp(appName);
        return 0;
    }

    if (arg1 == "--processid") {
        TRACELIB_NAMESPACE_IDENT(test_getCurrentProcessId)();
    } else if (arg1 == "--threadid") {
        TRACELIB_NAMESPACE_IDENT(test_getCurrentThreadId)();
    } else if (arg1 == "--starttime") {
        TRACELIB_NAMESPACE_IDENT(test_getCurrentProcessStartTime)();
    } else {
        cout << appName << ": Unknown option '" << arg1 << "'" << endl;
        suggestHelp(appName);
        return 1;
    }

    cout << g_verificationCount << " verifications; "
         << g_failureCount << " failures found." << endl;
    return g_failureCount;
}
