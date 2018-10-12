#!/usr/bin/env bash
set -x
if [[ "x$1" == "x" ]] ; then
    echo "Usage: install-requirements.sh [osx|linux]"
    exit 1
fi

if [[ "$1" == "osx" ]] ; then
    brew update
    brew install doxygen qt5 ninja
else
    docker pull froglogic/tracetool:latest
fi
