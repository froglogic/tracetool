#!/usr/bin/env python

# Simple script to do CI builds and package builds of tracelib
# on different platforms and avoid putting the platform-specific
# knowledge into the CI system.

import os
import re
import shutil
import subprocess
import tempfile
import sys
import ctypes


is_windows = sys.platform.startswith("win")
is_mac = sys.platform.startswith("darwin")
arch = None
compiler = None

# Make stdout line-buffered so any print()'s end up in the
# output directly and no buffering occurs
# stderr is line-buffered by default already so no reason to do that there
if not is_windows:
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 1)
def myprint(txt):
    print txt
    if is_windows:
        # On Windows line-buffering is the same as full buffering, so need to override
        # print and explicitly flush there
        sys.stdout.flush()

def long_path_name(path):
    if is_windows:
        buf = ctypes.create_unicode_buffer(260)
        GetLongPathName = ctypes.windll.kernel32.GetLongPathNameW
        rv = GetLongPathName(unicode(path), buf, 260)
        myprint("retrieved long pathname for %s? %s -> %s" % (path, rv, buf.value))
        if rv != 0 and rv <= 260:
            return buf.value
    return path

def find_exe_in_path(exeBaseName, env=os.environ):
    binary = bin_name(exeBaseName)
    for path in env["PATH"].split(os.pathsep):
        abspath = os.path.join(path, binary)
        if os.path.exists(abspath) and os.access(abspath, os.X_OK):
            return abspath
    myprint("%s not found in paths: %s" %(binary, env["PATH"]))
    return None


# Taken from buildsquish
def compileEnvFromBatchFile(batchDict):
    def dictKeysToUpper(d):
        d2 = {}
        for k, v in d.iteritems():
            d2[k.upper()] = v
        return d2

    def getEnvFromBatchFile(script):
        p = subprocess.Popen([os.path.expandvars(script), '>', 'NUL', '&&', 'set'],
                             shell=True,
                             stdout=subprocess.PIPE)
        stdout, stderr = p.communicate()
        pairs = [line.split('=', 1) for line in stdout.splitlines(False)]
        return dictKeysToUpper(dict(pairs))

    return lambda arch, compiler: getEnvFromBatchFile(batchDict[arch][compiler])


def fetch_run_environment(arch, compiler):
    if is_windows:
        compilerBatchFiles = {
                "x86": {
                    "mingw" : "C:\\MinGW-5.1.6\\bin\\env.bat",
                    "msvc6" : "%PROGRAMFILES(X86)%\\Microsoft Visual Studio\\VC98\\Bin\\vcvars32.bat",
                    "msvc7" : "%VS71COMNTOOLS%\\..\\..\\Vc7\\bin\\vcvars32.bat",
                    "msvc8" : "%VS80COMNTOOLS%\\..\\..\\VC\\bin\\vcvars32.bat",
                    "msvc9" : "%VS90COMNTOOLS%\\..\\..\\VC\\bin\\vcvars32.bat",
                    "msvc10": "%VS100COMNTOOLS%\\..\\..\\VC\\bin\\vcvars32.bat",
                    "msvc11": "%VS110COMNTOOLS%\\..\\..\\VC\\bin\\vcvars32.bat",
                    "msvc12": "%VS120COMNTOOLS%\\..\\..\\VC\\bin\\vcvars32.bat",
                    },
                "x64": {
                    "msvc8" : "%VS80COMNTOOLS%\\..\\..\\VC\\bin\\amd64\\vcvarsamd64.bat",
                    "msvc9" : "%VS90COMNTOOLS%\\..\\..\\VC\\bin\\amd64\\vcvarsamd64.bat",
                    "msvc10": "%VS100COMNTOOLS%\\..\\..\\VC\\bin\\amd64\\vcvars64.bat",
                    "msvc11": "%VS110COMNTOOLS%\\..\\..\\VC\\bin\\amd64\\vcvars64.bat",
                    "msvc12": "%VS120COMNTOOLS%\\..\\..\\VC\\bin\\amd64\\vcvars64.bat",
                    },
                }
        if os.path.exists(compilerBatchFiles[arch][compiler]):
            return compileEnvFromBatchFile(compilerBatchFiles)(arch, compiler)
    else:
        # Search compiler in PATH
        for path in os.environ["PATH"].split(":"):
            if os.path.exists(os.path.join(path, compiler)):
                newenv = dict(os.environ)
                newenv['CXX'] = 'g++-4.1'
                newenv['CC'] = 'gcc-4.1'
                return newenv
    raise Exception("Compiler/Arch %s/%s not found" % (compiler, arch))



def bin_name(basename):
    return basename + ".exe" if is_windows else basename


def tryCompile(compiler):
    myprint("Compiler        : %s" % compiler)
    myprint("Architecture    : %s" % arch)
    run_env = fetch_run_environment(arch, compiler)

    myprint("\nCalling %s\n" % "\n ".join(cmake_args))
    subprocess.check_call(cmake_args, env=run_env, cwd=builddir)

def main():
    global arch, compiler

    progpath = os.path.dirname(os.path.realpath(__file__))
    srcdir = os.path.realpath(progpath)

    if is_windows:
        # No MinGW since the MSVC-built tracelib is not compatible with mingw
        compilers = ["msvc6", "msvc7", "msvc8", "msvc9", "msvc10", "msvc11", "msvc12"]
    elif is_mac:
        compilers = ["g++-4.0", "g++-4.2", "clang"]
    else:
        compilers = ["g++-%s" % ver for ver in ["4.1", "4.2", "4.3", "4.4", "4.5", "4.6", "4.7", "4.8"]]
    if is_windows or is_mac:
        architectures = ["x86", "x64"]
    else
        architectures = ["native"]

    for arch in architectures:
        for compiler in compilers:
            tryCompile(compiler, arch)

if __name__ == "__main__":
    sys.exit(main())

