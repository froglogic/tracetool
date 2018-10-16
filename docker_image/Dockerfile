# Build docker image to generate linux packages for tracetool
# Uses Qt from binary installer from TQC to get better relocatable
# Qt, based on the ubuntu image
# Also includes all dev-tools for building tracetool

FROM ubuntu:16.04

ARG QT_VERSION=5.9.6
ENV DEBIAN_FRONTEND noninteractive
ENV QT_PATH /opt/Qt

RUN apt update && apt full-upgrade -y

# Qt dependencies and stuff needed to fetch Qt
RUN apt install -y --no-install-recommends \
    curl ca-certificates libglib2.0-0 libx11-xcb1 libgl1 libfontconfig1 libdbus-1-3 libsm6 libxi6

# Dependencies for building tracetool
RUN apt install -y --no-install-recommends \
    cmake ninja-build binutils-dev libiberty-dev g++ doxygen mesa-common-dev

# Taken from https://github.com/benlau/qtci/blob/master/bin/extract-qt-installer#4991cf6
COPY extract-qt-installer.sh /opt

RUN curl -Lo /opt/installer.run https://download.qt.io/official_releases/qt/$(echo "${QT_VERSION}" | cut -d. -f 1-2)/${QT_VERSION}/qt-opensource-linux-x64-${QT_VERSION}.run &&\
    QT_CI_PACKAGES=qt.$(echo "$QT_VERSION" | tr -d .).gcc_64 /opt/extract-qt-installer.sh /opt/installer.run $QT_PATH && \
    rm -r /opt/installer.run /opt/extract-qt-installer.sh && \
    rm -r $QT_PATH/InstallationLog.txt $QT_PATH/MaintenanceTool* $QT_PATH/Tools $QT_PATH/components.xml \
        $QT_PATH/dist $QT_PATH/network.xml $QT_PATH/Docs $QT_PATH/Examples

WORKDIR /tracer

