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
# FIXME: Move path variables into into some kind of PathGenerator class?
binpkg = os.path.expanduser(os.path.join("S:\\" if is_windows else "~", "binPackage"))
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
        return compileEnvFromBatchFile( {
                "x86": {
                    "msvc9" : "%VS90COMNTOOLS%\\..\\..\\VC\\bin\\vcvars32.bat",
                    },
                "x64": {
                    "msvc9" : "%VS90COMNTOOLS%\\..\\..\\VC\\bin\\amd64\\vcvarsamd64.bat",
                    },
                })(arch, compiler)
    elif not is_mac:
        # Linux
        for path in os.environ["PATH"].split(":"):
            if os.path.exists(os.path.join(path, "gcc-4.1")) and os.path.exists(os.path.join(path, "g++-4.1")):
                newenv = dict(os.environ)
                newenv['CXX'] = 'g++-4.1'
                newenv['CC'] = 'gcc-4.1'
                return newenv
        # No gcc 4.1, so lets use whatever is in path as fallback to run ci-builds locally and test the script
        return None
    else:
        return None


def verify_path(path):
    if not os.path.exists(path):
        raise Exception("Missing %s for CI build" % path)
    return path


def package_path(pkg_name, pkg_compiler = None, pkg_version = None):
    global arch
    assert(arch != None)
    if is_windows:
        pp = os.path.join(binpkg, pkg_name, arch)
    else:
        pp = os.path.join(binpkg, arch, pkg_name)

    # Compiler dir is optional, many packages lack one
    if pkg_compiler:
        pp = os.path.join(pp, pkg_compiler)

    if pkg_version:
        pp = os.path.join(pp, pkg_version)

    return verify_path(pp)


def bin_name(basename):
    return basename + ".exe" if is_windows else basename


def binpkg_arch_path(pkg, arch, ver):
    if is_windows:
        return os.path.join(binpkg, pkg, arch, ver)
    return os.path.join(binpkg, arch, pkg, ver)

def doxygen_path():
    if is_windows:
        return os.path.join(binpkg_arch_path("doxygen", "x86", ""), bin_name("doxygen"))
    elif is_mac:
        return os.path.join("/Applications/Utilities/Doxygen.app/Contents/Resources/doxygen")
    else:
        return "/usr/bin/doxygen"

def qt_base_path():
    if is_windows:
        return "F:\\"
    else:
        return os.path.expanduser(os.path.join("~", "Qt"))


def qmake_path(qtver):
    global arch, compiler
    if is_windows:
        return os.path.join(qt_base_path(), compiler, arch, qtver, "bin", "qmake.exe")
    elif is_mac:
        return os.path.join(qt_base_path(), arch, qtver+"-frameworks", "bin", "qmake")
    else:
        return os.path.join(qt_base_path(), arch, qtver, "bin", "qmake")

def parseArchitecture(arch):
    if arch in ["x86", "x64"]:
        return arch
    raise Exception("Unknown architecture %s (supported: x86, x64)" % arch)

def compiler():
    if is_windows:
        return "msvc9"
    else:
        return "gcc"

def qt_version():
    if is_windows:
        return "4.8.0"
    elif is_mac:
        # does not matter at the moment, see qt_path
        return "4.8.7"
    else:
        return "4.8.0"

def getExecutableFromCMake(executableName):
    if 'CMAKE_PATH' in os.environ:
        return os.path.join(os.environ['CMAKE_PATH'], 'bin', executableName)
    return find_exe_in_path(executableName)

def main():
    global arch, compiler

    progpath = os.path.dirname(os.path.realpath(__file__))
    srcdir = os.path.realpath(progpath)

    compiler = compiler()
    qtver = qt_version()

    if len(sys.argv) < 2:
        myprint("Usage: %s arch [package]" % sys.argv[0])
        return 1

    arch = parseArchitecture(sys.argv[1])
    do_package = len(sys.argv) > 2 and sys.argv[2] == 'package'

    buildtype = "ci" if not do_package else "pkg"
    builddir_name = "%sbuild_%s" % (buildtype, arch)
    builddir = os.path.realpath(os.path.join(srcdir, builddir_name))

    myprint("Compiler        : %s" % compiler)
    myprint("Architecture    : %s" % arch)
    myprint("Qt Version      : %s" % qtver)
    myprint("Source directory: %s" % srcdir)
    myprint("Build directory : %s" % builddir)

    if do_package and os.path.exists(builddir):
        myprint("Removing build directory to get a clean package build")
        shutil.rmtree(builddir)

    if not os.path.exists(builddir):
        myprint("Creating missing build directory")
        os.mkdir(builddir)
    os.chdir(builddir)

    verify_path(binpkg)

    cmake_args = [getExecutableFromCMake('cmake'), '-G']

    if is_windows:
        cmake_args.append("NMake Makefiles")
    else:
        cmake_args.append("Unix Makefiles")

    run_env = fetch_run_environment(arch, compiler)

    # For CI-builds use a somewhat-fast build with debug symbols for the
    # potential test execution that follows to get useful crash info
    if do_package:
        cmake_args.append("-DCMAKE_BUILD_TYPE=Release")
    else:
        cmake_args.append("-DCMAKE_BUILD_TYPE=RelWithDebInfo")
    # on Linux and Windows we specify a special bitness suffix
    # so both architectures can be put into the same directory.
    # Used for packaging and ci since the ci-artifacts are
    # used for a job which tries to compile for multiple archs
    # on at least windows
    if is_windows or not is_mac:
        cmake_args.append("-DARCH_LIB_SUFFIX=%s" % arch)

    packageInWindowsTemp = do_package and is_windows
    packagingDir = ""
    if packageInWindowsTemp:
        # Make sure to use a short path on Windows for CPack so it does not run into
        # the maximum path length. Can happen especially with jenkins nested paths
	# On Windows gettempdir() may return a path with 8.3 directory names
	# but cmake will eventually compare that (using strequal) against a
	# long pathname, so convert the directory to long pathname as well
        packagingDir = os.path.join(long_path_name(tempfile.gettempdir()), "tracelib-%s" % builddir_name)
        if os.path.exists(packagingDir):
            shutil.rmtree(packagingDir)
        os.makedirs(packagingDir)
        cmake_args.append("-DCPACK_PACKAGE_DIRECTORY=%s" % packagingDir.replace("\\", "\\\\"))

    qmake_exe = verify_path(qmake_path(qtver))
    cmake_args.append("-DDOXYGEN_EXECUTABLE=%s" % verify_path(doxygen_path()))
    cmake_args.append("-DQT_QMAKE_EXECUTABLE=%s" % qmake_exe)
    cmake_args.append("-DCMAKE_INSTALL_PREFIX=%s" % os.path.join(builddir, "..", "install"))

    if is_windows:
        run_env["PATH"] = os.path.split(qmake_exe)[0] + os.pathsep + run_env["PATH"]
        myprint("Path for running tests: %s" % run_env["PATH"])

    cmake_args.append(srcdir)

    myprint("\nCalling %s\n" % "\n ".join(cmake_args))
    subprocess.check_call(cmake_args, env=run_env, cwd=builddir)

    env_to_search = run_env if is_windows else os.environ
    make_args = [find_exe_in_path("nmake" if is_windows else "make", env_to_search)]

    if do_package:
        make_args.append("preinstall")
    else:
        make_args.append("all")
        # Install to make it easier to archive some of the installation for other jenkins jobs
        make_args.append("install")

    myprint("\nCalling %s\n" % "\n ".join(make_args))
    subprocess.check_call(make_args, env=run_env, cwd=builddir)

    if do_package:
        cpack_args = [getExecutableFromCMake('cpack'), "-V", "--config", "CPackConfig.cmake"]
        myprint("\nCalling %s\n" % "\n ".join(cpack_args))
        subprocess.check_call(cpack_args, env=run_env, cwd=builddir)
    else:
        # cleanup existing Testing/ directory since otherwise the ci-tools may pick up
        # old test files
        testingDir = os.path.join(builddir, "Testing")
        myprint("Cleaning up %s if it exists: %s" %(testingDir, os.path.exists(testingDir)))
        if os.path.exists(testingDir):
            shutil.rmtree(testingDir)
        ctest_args = [getExecutableFromCMake('ctest'), "--no-compress-output", "-T", "Test"]
        myprint("\nCalling %s\n" % "\n ".join(ctest_args))
        subprocess.call(ctest_args, env=run_env, cwd=builddir)

    if packageInWindowsTemp:
        # Now move the generated artefacts to our builddir so jenkins can pick them up
        pkgnamere = re.compile("tracelib-.*Windows.*zip")
        for fn in filter(lambda x: pkgnamere.match(x) is not None, os.listdir(packagingDir)):
            shutil.move(os.path.join(packagingDir, fn), os.path.join(builddir, fn))
        shutil.rmtree(packagingDir)


if __name__ == "__main__":
    sys.exit(main())

