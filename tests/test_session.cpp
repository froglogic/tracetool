/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>
#include <iostream>

#include "../gui/columnsinfo.h"

using namespace std;

static void printHelp(const char *appName)
{
    cout << "Usage: " << appName << " [OPTION]" << endl
         << "Possible options are:" << endl
         << "\t--columns"
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


static void test_columnInfo()
{
    {
        // Most simplest case
        ColumnsInfo info;
        QVariant data = info.sessionState();
        bool success = info.restoreSessionState(data);
        assertTrue("Restore default config", success);
    }

    {
        ColumnsInfo info;
        QList<int> visible, invisible;
        visible << 1 << 0;
        for (int i = 2; i < info.columnCount(); ++i) {
            invisible.append(i);
        }
        info.setSorting(visible, invisible);
        QVariant data = info.sessionState();

        bool success = info.restoreSessionState(data);
        assertTrue("Restore modified config", success);
        QList<int> visibleRestored = info.visibleColumns();
        QList<int> invisibleRestored = info.invisibleColumns();
        assertTrue("Visible list restored", visibleRestored == visible);
        assertTrue("Invisible list restored", invisibleRestored == invisible);
    }
}
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

    if (arg1 == "--columns") {
        test_columnInfo();
    } else {
        cout << appName << ": Unknown option '" << arg1 << "'" << endl;
        suggestHelp(appName);
        return 1;
    }

    cout << g_verificationCount << " verifications; "
         << g_failureCount << " failures found." << endl;
    return g_failureCount;
}
