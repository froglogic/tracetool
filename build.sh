#!/usr/bin/env bash
set -x
if [[ "x$1" == "x" ]] ; then
    echo "Usage: build.sh [osx|linux] [tagname]"
    exit 1
fi

buildType=""
bundleQt="OFF"
enableInstallRpath="OFF"
rpath=""
deploymentTarget=""
qtpath=""
if [[ "$1" == "osx" ]] ; then
    qtpath="/usr/local/opt/qt5"
else
    qtpath="/opt/Qt/5.9.6/gcc_64"
fi
mkdir build
if [[ "x$2" == "x" ]] ; then
    buildType="Debug"
else
    bundleQt="ON"
    buildType="Release"
    if [[ "$1" == "linux" ]] ; then
        rpath="\\\$ORIGIN/../lib"
    else
        deploymentTarget="10.10"
    fi
    buildTypeArg="Release"
fi
cmake_args="-DCMAKE_PREFIX_PATH=$qtpath -DCMAKE_BUILD_TYPE=${buildType} -DBUNDLE_QT=${bundleQt} -DENABLE_INSTALL_RPATH=${enableInstallRpath} -DCMAKE_INSTALL_RPATH=${rpath} -DCMAKE_OSX_DEPLOYMENT_TARGET=$deploymentTarget -G Ninja"

if [[ "$1" == "osx" ]] ; then
    pushd build
    cmake ${cmake_args} ..
    ninja
    ninja test
    ninja package
    popd
else
    docker run -u $UID -v $PWD:/tracer froglogic/tracetool:latest /bin/bash -c "cd build && cmake ${cmake_args} .. && ninja && ninja test && ninja package"
fi
