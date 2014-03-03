#!/usr/bin/env python

# Simple script to do CI builds and package builds of tracelib
# on different platforms and avoid putting the platform-specific
# knowledge into the CI system.

import os
import re
import shutil
import subprocess
import platform
import tempfile
import sys
import ctypes


is_windows = sys.platform.startswith("win")
is_mac = sys.platform.startswith("darwin")
arch = None
compiler = None

class CompilerNotFoundException(Exception):
    def __init__(self, compiler, arch):
        Exception.__init__(self, "Could not found compiler: %s for architecture: %s" %(compiler, arch))

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
        if os.path.exists(os.path.expandvars(compilerBatchFiles[arch][compiler])):
            return compileEnvFromBatchFile(compilerBatchFiles)(arch, compiler)
    else:
        # Search compiler in PATH
        for path in os.environ["PATH"].split(":"):
            if os.path.exists(os.path.join(path, compiler)):
                newenv = dict(os.environ)
                newenv['CXX'] = compiler
                newenv['CC'] = compiler
                return newenv
    raise CompilerNotFoundException(compiler, arch)



def bin_name(basename):
    return basename + ".exe" if is_windows else basename

def compilerForPlatform(env):
    if is_windows:
        return find_exe_in_path("cl", env)
    else:
        return find_exe_in_path(env["CXX"], env)

def tracelibLinkLibrary(tracelibbasedir, suffix):
    if is_windows:
        return os.path.join(tracelibbasedir, "lib", "tracelib%s.lib" % suffix)
    elif is_mac:
        return os.path.join(tracelibbasedir, "lib", "libtracelib.dylib")
    else:
        return os.path.join(tracelibbasedir, "lib", "libtracelib%s.so" % suffix)

def determineTracelibSuffix(arch):
    if arch == "native":
        return "x86" if platform.architecture()[0] == "32bit" else "x64"
    else:
        return arch

def tracelibArtifactsDir(arch, basedir):
    if is_windows:
        return os.path.join(basedir, "arch=%s,nodelimit=tracelib,os=windows" % arch, "install")
    elif is_mac:
        return os.path.join(basedir, "arch=%s,nodelimit=tracelib,os=macosx" % arch, "install")
    else:
        return os.path.join(basedir, "arch=%s,nodelimit=tracelib,os=linux" % arch, "install")

def tryCompile(compiler, arch, tracelibbasedir, srcdir):
    myprint("Compiler        : %s" % compiler)
    myprint("Architecture    : %s" % arch)
    try:
        run_env = fetch_run_environment(arch, compiler)

        tracelibsuffix = determineTracelibSuffix(arch)
        tracelibartifacts = tracelibArtifactsDir(tracelibsuffix, tracelibbasedir)

        compilerargs = [compilerForPlatform(run_env),
                        "-I", os.path.join(tracelibartifacts, "include"),
                        tracelibLinkLibrary(tracelibartifacts, tracelibsuffix)]
        if is_windows:
            compilerargs.append("/EHsc")
        if is_mac:
            if arch == "x86":
                compilerargs.append("-m32")
            else:
                compilerargs.append("-m64")
        if is_windows:
            compilerargs.append("/Fe" + os.path.join(srcdir, bin_name("compiletest")))
        else:
            compilerargs.append("-o")
            compilerargs.append(os.path.join(srcdir, bin_name("compiletest")))
        compilerargs.append(os.path.join(srcdir, "compiletest.cpp"))
        myprint("\nCalling %s\n" % "\n ".join(compilerargs))
        subprocess.check_call(compilerargs, env=run_env, cwd=srcdir)
    except CompilerNotFoundException, e:
        myprint("Could not test: %s" % e)
    except subprocess.CalledProcessError, e:
        myprint("Error compiling: %s" % e)
        return False
    return True

def main():
    global arch, compiler

    progpath = os.path.dirname(os.path.realpath(__file__))
    srcdir = os.path.realpath(progpath)

    if len(sys.argv) < 2:
        print "Usage: %s <tracelibinstalldir>" % sys.argv[0]
        return 2

    if is_windows:
        # No MinGW since the MSVC-built tracelib is not compatible with mingw
        compilers = ["msvc6", "msvc7", "msvc8", "msvc9", "msvc10", "msvc11", "msvc12"]
    elif is_mac:
        compilers = ["g++-4.0", "g++-4.2", "clang++"]
    else:
        compilers = ["g++-%s" % ver for ver in ["4.1", "4.2", "4.3", "4.4", "4.5", "4.6", "4.7", "4.8"]]
    if is_windows:
        architectures = ["x86", "x64"]
    elif is_mac:
        architectures = ["x86"]
    else:
        architectures = ["native"]

    compiledSuccessfully = True
    for arch in architectures:
        for compiler in compilers:
            if not is_windows or arch != 'x64' or compiler not in ['msvc6', 'msvc7']:
                if not tryCompile(compiler, arch, sys.argv[1], srcdir):
                    compiledSuccessfully = False
    return 0 if compiledSuccessfully else 1

if __name__ == "__main__":
    sys.exit(main())

