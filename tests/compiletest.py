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
import difflib

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

def tracelibInstallDir(arch, basedir):
    tracelibbasedir = tracelibArtifactsDir(determineTracelibSuffix(arch), basedir)
    if not os.path.exists(tracelibbasedir):
        tracelibbasedir = basedir
    return tracelibbasedir

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
    compiletestexe = bin_name("compiletest")
    for name in ["compiletest%s" % suffix for suffix in [".pdb", ".suo", ".obj", "", ".exe", ".ilk"]]:
        if os.path.exists(os.path.join(srcdir, name)):
            abspath = os.path.join(srcdir, name)
            cnt = 0
            while not os.access(abspath, os.W_OK) and cnt < 5:
                time.sleep(.2)
            os.remove(abspath)
    try:
        run_env = fetch_run_environment(arch, compiler)

        tracelibsuffix = determineTracelibSuffix(arch)
        if (not os.path.exists(tracelibLinkLibrary(tracelibbasedir, tracelibsuffix))
            and os.path.exists(tracelibLinkLibrary(tracelibbasedir, ""))):
            tracelibsuffix = ""


        compilerargs = [compilerForPlatform(run_env),
                        "-I", os.path.join(tracelibbasedir, "include"),
                        tracelibLinkLibrary(tracelibbasedir, tracelibsuffix)]
        if is_windows:
            compilerargs.append("/EHsc")
        if is_mac:
            if arch == "x86":
                compilerargs.append("-m32")
            else:
                compilerargs.append("-m64")
        if is_windows:
            compilerargs.append("/Z7")
            compilerargs.append("/Fe" + os.path.join(srcdir, compiletestexe))
        else:
            compilerargs.append("-g")
            compilerargs.append("-o")
            compilerargs.append(os.path.join(srcdir, compiletestexe))
        compilerargs.append(os.path.join(srcdir, "compiletest.cpp"))
        myprint("\nCalling %s\n" % "\n ".join(compilerargs))
        subprocess.check_call(compilerargs, env=run_env, cwd=srcdir)
    except CompilerNotFoundException, e:
        myprint("Could not test: %s" % e)
        return (True, False)
    except subprocess.CalledProcessError, e:
        myprint("Error compiling: %s" % e)
        return (False, False)
    return (True, True)

def verifyOutput(compiler, arch, srcdir, tracelibdir):
    compiletestexe = bin_name("compiletest")
    compiletestlog = os.path.join(srcdir, "compiletest.log")
    if not os.path.exists(os.path.join(srcdir, compiletestexe)):
        # this is only called for successful compiles, so there should better be an executable
        return False
    if os.path.exists(os.path.join(compiletestlog)):
        os.remove(compiletestlog)
    open(os.path.join(srcdir, "tracelib.xml"), "w").write("""
<tracelibConfiguration>
<process>
    <name>%s</name>
    <output type="file">
        <option name="filename">%s</option>
    </output>
    <serializer type="xml">
        <option name="beautifiedOutput">yes</option>
    </serializer>
    <tracepointset variables="yes">
      <matchallfilter />
    </tracepointset>
</process>
</tracelibConfiguration>
""" % (compiletestexe, compiletestlog))
    env = os.environ
    if is_windows:
        env["PATH"] += os.pathsep + os.path.join(tracelibdir, "bin")
    elif is_mac:
        env["DYLD_LIBRARY_PATH"] = os.path.join(tracelibdir, "lib")
    else:
        env["LD_LIBRARY_PATH"] = os.path.join(tracelibdir, "lib")
    subprocess.check_call([os.path.join(srcdir,compiletestexe)], env=env)
    actualXml = open(os.path.join(srcdir, "compiletest.log"), "r").read()
    replacements = [re.compile(r'(pid)="[0-9]+"'),
                    re.compile(r'(process_starttime)="[0-9]+"'),
                    re.compile(r'(time)="[0-9]+"'),
                    re.compile(r'(tid)="[0-9]+"')]
    for repl in replacements:
        actualXml = repl.sub(r"\1=\"\1\"", actualXml)
    actualXml = re.sub(r"<stackposition>[0-9]+", r"<stackposition>1", actualXml)
    actualXml = re.sub(r"(<location lineno=\"[0-9]+\"><!\[CDATA\[)[^\]]+\]\]>", r"\1compiletest.cpp]]>", actualXml)
    if is_windows:
        actualXml = re.sub(r"<processname><!\[CDATA\[compiletest\.exe]", r"<processname><![CDATA[compiletest]", actualXml)
        actualXml = re.sub(r"<shutdownevent([^>]+)><!\[CDATA\[compiletest\.exe]", r"<shutdownevent\1><![CDATA[compiletest]", actualXml)
    expectedXml = open(os.path.join(srcdir, "compiletest_expected.log"), "r").read()
    if compiler == "msvc6":
        expectedXml = re.sub(r"<function>.*</function>", r"<function><![CDATA[unknown]]></function>", expectedXml)
    elif compiler.startswith("msvc"):
        actualXml = re.sub(r"<function>(.+) __cdecl (.+)</function>", r"<function>\1 \2</function>", actualXml)
        actualXml = re.sub(r"<function>(.+)\(void\)(.+)</function>", r"<function>\1()\2</function>", actualXml)
    if actualXml == expectedXml:
        return True
    print "Difference in expected log:\n  %s" % ("\n  ".join(difflib.unified_diff(expectedXml.split("\n"), actualXml.split("\n"))), )
    return False

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
    ranSuccessfully = True
    for arch in architectures:
        for compiler in compilers:
            if not is_windows or arch != 'x64' or compiler not in ['msvc6', 'msvc7']:
                tracelibdir = tracelibInstallDir(arch, sys.argv[1])
                compileResult = tryCompile(compiler, arch, tracelibdir, srcdir)
                if not compileResult[0]:
                    compiledSuccessfully = False
                elif compileResult[1]:
                    # Only run this if tryCompile found a compiler and compiled something
                    if not verifyOutput(compiler, arch, srcdir, tracelibdir):
                        ranSuccessfully = False
    return 0 if (compiledSuccessfully and ranSuccessfully) else 1

if __name__ == "__main__":
    sys.exit(main())

