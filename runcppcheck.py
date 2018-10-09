#!/usr/bin/env python

import os
import datetime
import subprocess
import sys

def main():
    undefines = [
            "NDEBUG",
            "Q_CC_XLC",
            "Q_OS_HPUX",
            "Q_OS_IRIX",
            "Q_OS_SOLARIS",
            "WINCE",
            "_AIX",
            "_DEBUG",
            "_LP64",
            "__BORLANDC__",
            "__INTEL_COMPILER",
            "__LP64__",
            "__Lynx__",
            "__QNX__",
            "__SunOS_5_10",
            "__sgi",
            "hpux",
            ]

    defines = [
            "__cplusplus",
            ]

    ignore_paths = [
            "3rdparty/pcre-8.10",
            ]

    include_paths = [
            "hooklib",
            "server",
            "gui",
            ]

    check_paths = [
            "hooklib",
            "server",
            "gui",
            "tests"
            "examples",
            "convertdb",
            "trace2xml",
            "xml2trace"
            ]

    cppcheckbinary = "cppcheck"
    if "CPPCHECK_PATH" in os.environ:
        cppcheckbinary = os.path.join(os.environ["CPPCHECK_PATH"], "cppcheck")
    cppcheck_args = [ cppcheckbinary,
            "--enable=all",
            "--quiet",
            "--xml",
            "--xml-version=2",
            ]
    cppcheck_args.extend(["-U" + u for u in undefines])
    cppcheck_args.extend(["-D" + d for d in defines])
    cppcheck_args.extend(["-i" + i for i in ignore_paths])
    cppcheck_args.extend(["-I" + i for i in include_paths])
    cppcheck_args.extend(check_paths)

    print("\nCalling %s\n" % "\n ".join(cppcheck_args))

    t_start = datetime.datetime.now()
    with open("cppcheck-results.xml", "w") as outfile:
        subprocess.check_call(cppcheck_args, stderr=outfile)
    t_elapsed = datetime.datetime.now() - t_start
    print("cppcheck took %d seconds" % t_elapsed.seconds)

if __name__ == "__main__":
    main()

