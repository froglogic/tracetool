#!/usr/bin/env python

# Simple script to do CI builds and package builds of tracelib
# on different platforms and avoid putting the platform-specific
# knowledge into the CI system.

import os
import shutil
import subprocess
import sys

is_windows = sys.platform.startswith("win")
is_mac = sys.platform.startswith("darwin")
# FIXME: Move path variables into into some kind of PathGenerator class?
binpkg = os.path.expanduser(os.path.join("S:\\" if is_windows else "~", "binPackage"))
arch = None
compiler = None

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
    if not is_windows:
        return None
    return compileEnvFromBatchFile( {
            "x86": {
                "msvc9" : "%VS90COMNTOOLS%\\..\\..\\VC\\bin\\vcvars32.bat",
                },
            "x64": {
                "msvc9" : "%VS90COMNTOOLS%\\..\\..\\VC\\bin\\amd64\\vcvarsamd64.bat",
                },
            })(arch, compiler)


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
        return os.path.join(package_path("doxygen"), bin_name("doxygen"))
    elif is_mac:
        return os.path.join("/Applications/Utilities/Doxygen.app/Contents/Resources/doxygen")
    else:
        return "/usr/bin/doxygen"

def qt_base_path():
    if is_windows:
        return "Q:\\"
    else:
        return os.path.expanduser(os.path.join("~", "Qt"))


def qmake_path(qtver):
    global arch, compiler
    if is_windows:
        return os.path.join(qt_base_path(), compiler, arch, qtver, "bin", "qmake.exe")
    elif is_mac:
        return "/Users/andreas/qt/4.8.5-full/bin/qmake"
    else:
        return os.path.join(qt_base_path(), arch, qtver, "bin", "qmake")

def parseArchitecture(arch):
    if arch in ["x86", "x64", "universal"]:
        return arch
    raise Exception("Unknown architecture %s (supported: x86, x64, universal)" % arch)

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
        return "4.8.0"
    else:
        return "4.8.0"

def main():
    global arch, compiler

    progpath = os.path.dirname(os.path.realpath(__file__))
    srcdir = os.path.realpath(progpath)

    compiler = compiler()
    qtver = qt_version()

    if len(sys.argv) < 2:
        print "Usage: %s arch" % sys.argv[0]
        return 1

    arch = parseArchitecture(sys.argv[1])
    do_package = len(sys.argv) > 1 and sys.argv[1] == 'package'

    buildtype = "ci" if not do_package else "pkg"
    builddir = os.path.realpath(os.path.join(srcdir, "%sbuild_%s_%s_%s" % (buildtype, compiler, arch, qtver)))

    print("Compiler        : %s" % compiler)
    print("Architecture    : %s" % arch)
    print("Qt Version      : %s" % qtver)
    print("Source directory: %s" % srcdir)
    print("Build directory : %s" % builddir)

    if do_package and os.path.exists(builddir):
        print("Removing build directory to get a clean package build")
        shutil.rmtree(builddir)

    if not os.path.exists(builddir):
        print("Creating missing build directory")
        os.mkdir(builddir)
    os.chdir(builddir)

    verify_path(binpkg)

    cmake_args = [bin_name("cmake")]

    cmake_args.append("-G")
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

    # Do a multi-arch binary on MacOSX
    if do_package and arch=='universal':
        cmake_args.append("-DCMAKE_OSX_ARCHITECTURES=i386;x86_64")

    cmake_args.append("-DDOXYGEN_EXECUTABLE=%s" % verify_path(doxygen_path()))
    cmake_args.append("-DQT_QMAKE_EXECUTABLE=%s" % verify_path(qmake_path(qtver)))

    cmake_args.append(srcdir)

    print("\nCalling %s\n" % "\n ".join(cmake_args))
    subprocess.check_call(cmake_args, env=run_env, cwd=builddir)

    make_args = [bin_name("cmake"), "--build", builddir]

    if do_package:
        make_args.append("--target")
        make_args.append("package")
    else:
        make_args.append("--")
        make_args.append("all")
        make_args.append("test")

    print("\nCalling %s\n" % "\n ".join(make_args))
    subprocess.check_call(make_args, env=run_env, cwd=builddir)

if __name__ == "__main__":
    sys.exit(main())

