/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "../convertdb/getopt.h"

#include "../core/tracelib.h"
#include "../server/database.h"

#include <cstdio>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

namespace Error
{
    const int None = 0;
    const int CommandLineArgs = 1;
    const int Open = 2;
    const int File = 3;
    const int Transformation = 4;
}

static void printHelp(const QString &app)
{
    fprintf(stdout, "Usage: %s [options] DATABASE\n"
            "Options:\n"
            "  -o, --output FILE   Writes XML to FILE\n"
	    "  --help              Print this help\n"
	    "\n", qPrintable(app));
}

//TODO: remove codedupl./copied from entryitemmodel.cpp
static QString tracePointTypeAsString(int i)
{
    // ### could do some caching here
    // ### assert range - just in case
    using TRACELIB_NAMESPACE_IDENT(TracePointType);
    TracePointType::Value t = static_cast<TracePointType::Value>(i);
    QString s = TracePointType::valueAsString(t);
    return s;
}

static QString variableTypeAsString(int i)
{
    using TRACELIB_NAMESPACE_IDENT(VariableType);
    VariableType::Value t =  static_cast<VariableType::Value>(i);
    QString s = VariableType::valueAsString(t);
    return s;
}

static bool toXml(const QSqlDatabase db, FILE *output, QString *errMsg)
{
    using TRACELIB_NAMESPACE_IDENT(TracePointType);

    const char header[] =
        "<?xml version='1.0'?>\n"
        "<!DOCTYPE trace [\n"
        "  <!ELEMENT trace (traceentry*)>\n"
        "  <!ELEMENT traceentry (timestamp, process, threadid,\n"
        "                        tracepoint, message, stackposition,\n"
        "                        variables?)>\n"
        "  <!ATTLIST traceentry id CDATA #REQUIRED\n"
        "                       type CDATA #REQUIRED>\n"
        "  <!ELEMENT timestamp (#PCDATA)>\n"
        "  <!ELEMENT process (pid, name, starttime, endtime)>\n"
        "  <!ELEMENT pid (#PCDATA)>\n"
        "  <!ELEMENT name (#PCDATA)>\n"
        "  <!ELEMENT starttime (#PCDATA)>\n"
        "  <!ELEMENT endtime (#PCDATA)>\n"
        "  <!ELEMENT threadid (#PCDATA)>\n"
        "  <!ELEMENT tracepoint (pathname, line, function)>\n"
        "  <!ELEMENT pathname (#PCDATA)>\n"
        "  <!ELEMENT line (#PCDATA)>\n"
        "  <!ELEMENT function (#PCDATA)>\n"
        "  <!ELEMENT type (#PCDATA)>\n"
        "  <!ELEMENT message (#PCDATA)>\n"
        "  <!ELEMENT stackposition (#PCDATA)>\n"
        "  <!ELEMENT variables (variable)*>\n"
        "  <!ELEMENT variable (name, value, type)*>\n"
        "  <!ELEMENT value (#PCDATA)>\n"
        // name and type are already declared
        "]>\n"
        "<trace>\n";
    const char footer[] = "</trace>\n";
    const char traceentryTpl0[] =
        "  <traceentry id=\"%s\" type=\"%s\">\n"
        "    <timestamp>%s</timestamp>\n"
        "    <process>\n"
        "      <pid>%s</pid>\n"
        "      <name><![CDATA[%s]]></name>\n"
        "      <starttime>%s</starttime>\n"
        "      <endtime>%s</endtime>\n"
        "    </process>\n"
        "    <threadid>%s</threadid>\n"
        "    <tracepoint>\n"
        "      <pathname><![CDATA[%s]]></pathname>\n"
        "      <line>%s</line>\n"
        "      <function><![CDATA[%s]]></function>\n"
        "    </tracepoint>\n"
        "    <message><![CDATA[%s]]></message>\n"
        "    <stackposition>%s</stackposition>\n"
        "    <variables>\n";
    const char traceentryTpl1[] =
        "    </variables>\n"
        "  </traceentry>\n";
    const char variableTpl[] =
        "      <variable>\n"
        "        <name><![CDATA[%s]]></name>\n"
        "        <value><![CDATA[%s]]></value>\n"
        "        <type><![CDATA[%s]]></type>\n"
        "      </variable>\n";

    QSqlQuery getVariablesQuery(db);
    getVariablesQuery.prepare("SELECT"
                              " name,"
                              " value,"
                              " type "
                              "FROM"
                              " variable "
                              "WHERE"
                              " trace_entry_id = :trace_entry_id");

    QSqlQuery resultSet = db.exec("SELECT"
                                  " trace_entry.id,"
                                  " timestamp,"
                                  " process.name,"
                                  " process.pid,"
                                  " process.start_time,"
                                  " process.end_time,"
                                  " traced_thread.tid,"
                                  " path_name.name,"
                                  " trace_point.line,"
                                  " function_name.name,"
                                  " trace_point.type,"
                                  //" trace_point.verbosity,"
                                  " message, "
                                  " trace_entry.stack_position "
                                  "FROM"
                                  " trace_entry,"
                                  " trace_point,"
                                  " path_name, "
                                  " function_name, "
                                  " process, "
                                  " traced_thread "
                                  "WHERE"
                                  " trace_entry.trace_point_id = trace_point.id "
                                  "AND"
                                  " trace_point.function_id = function_name.id "
                                  "AND"
                                  " trace_point.path_id = path_name.id "
                                  "AND"
                                  " trace_entry.traced_thread_id = traced_thread.id "
                                  "AND"
                                  " traced_thread.process_id = process.id "
                                  "ORDER BY"
                                  " trace_entry.id");
    if (db.lastError().isValid()) {
        *errMsg = db.lastError().text();
        return false;
    }

    fprintf(output, "%s", header);

    while (resultSet.next()) {
        //TODO: reference fields by name
        fprintf(output, traceentryTpl0,
                resultSet.value(0).toString().toUtf8().constData(),
                tracePointTypeAsString(resultSet.value(10).toInt()).toUtf8().constData(), //type
                resultSet.value(1).toString().toUtf8().constData(),
                resultSet.value(3).toString().toUtf8().constData(), //pid
                resultSet.value(2).toString().toUtf8().constData(), //process name
                resultSet.value(4).toString().toUtf8().constData(),
                resultSet.value(5).toString().toUtf8().constData(),
                resultSet.value(6).toString().toUtf8().constData(),
                resultSet.value(7).toString().toUtf8().constData(),
                resultSet.value(8).toString().toUtf8().constData(),
                resultSet.value(9).toString().toUtf8().constData(),
                resultSet.value(11).toString().toUtf8().constData(),
                resultSet.value(12).toString().toUtf8().constData());

        if (resultSet.value(10).toInt() == TracePointType::Watch) {
            int traceEntryId = resultSet.value(0).toInt();
            getVariablesQuery.bindValue(0, traceEntryId);
            getVariablesQuery.exec();
            if (db.lastError().isValid()) {
                *errMsg = db.lastError().text();
                return false;
            }
            while (getVariablesQuery.next()) {
                fprintf(output, variableTpl,
                        getVariablesQuery.value(0).toString().toUtf8().constData(),
                        getVariablesQuery.value(1).toString().toUtf8().constData(),
                        variableTypeAsString(getVariablesQuery.value(2).toInt()).toUtf8().constData());
            }
            getVariablesQuery.finish();
        }

        fprintf(output, traceentryTpl1);
    }
    resultSet.finish();

    fprintf(output, "%s", footer);

    fflush( output );
    return true;
}

int main(int argc, char **argv)
{
    QCoreApplication a(argc, argv);

    GetOpt opt;
    bool help;
    QString traceFile, outputFile;
    opt.addSwitch("help", &help);
    opt.addOption('o', "output", &outputFile);
    opt.addArgument("traceDb", &traceFile);
    if (!opt.parse()) {
        if (help) {
            //TODO: bad, since GetOpt.parse() already prints
            //'Lacking required argument' warning
            printHelp(opt.appName());
            return Error::None;
        }
        fprintf(stderr, "Invalid command line argument. Try --help.\n");
	return Error::CommandLineArgs;
    }

    if (help) {
        printHelp(opt.appName());
        return Error::None;
    }

    QString errMsg;
    QSqlDatabase db = Database::open(traceFile, &errMsg);
    if (!db.isValid()) {
        fprintf(stderr, "Open error: %s\n", qPrintable(errMsg));
        return Error::Open;
    }

    FILE *outputStream;
    if (outputFile.isNull()) {
        outputStream = stdout;
    } else {
        outputStream = fopen(qPrintable(outputFile), "w");
        if (outputStream == NULL) {
            fprintf(stderr, "File '%s' cannot be opened for writing.\n", qPrintable(outputFile));
            return Error::File;
        }
    }

    if (!toXml(db, outputStream, &errMsg)) {
        fprintf(stderr, "Transformation error: %s\n", qPrintable(errMsg));
        return Error::Transformation;
    }
    fclose(outputStream);
    return Error::None;
}
