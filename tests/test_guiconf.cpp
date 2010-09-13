#include <QCoreApplication>
#include "../gui/configuration.h"

#include <string>
#include <iostream>

#include <QFile>

using namespace std;

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

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    if (argc != 2) {
        cout << "Missing file name" << endl;
        return 1;
    }
    const QString dir = QFile::encodeName(argv[1]);
    const QString fileName = dir + "/testconf.xml";

    QString errMsg;
    Configuration conf;
    if (!conf.load(fileName, &errMsg)) {
        cout << "Load error: " << qPrintable(errMsg) << endl;
        return 2;
    }

    assertEquals("Number of processes", conf.processCount(), 1);
    ProcessConfiguration *p0 = conf.process(0);
    assertTrue("Name of process", p0->m_name == "addressbook");
    assertTrue("Output type", p0->m_outputType == "tcp");
    assertTrue("Output host", p0->m_outputOption["host"] == "127.0.0.1");
    assertTrue("Serializer type", p0->m_serializerType == "xml");
    assertTrue("Serializer option",
               p0->m_serializerOption["beautifiedOutput"] == "yes");

    assertEquals("Number of tracepointsets", p0->m_tracePointSets.count(), 1);
    TracePointSet tps0 = p0->m_tracePointSets[0];
    assertEquals("Max verbosity", tps0.m_maxVerbosity, 1);
    
    fprintf(stdout, "OK.\n");
    return 0;
}
