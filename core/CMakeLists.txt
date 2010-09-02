INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE(CheckIncludeFile)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF(MSVC)

IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    SET(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES} /usr/local/include")
    INCLUDE_DIRECTORIES("/usr/local/include")
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

ADD_DEFINITIONS(-DPCRE_STATIC -DTIXML_USE_STL)

IF(NOT WIN32)
    # If we cannot use execinfo.h and backtrace() out of the box, try to find
    # libexecinfo which provides (roughly) the same functionality.
    TRY_COMPILE(HAS_EXECINFO
                ${PROJECT_BINARY_DIR}/config-tests
                ${PROJECT_SOURCE_DIR}/config-tests/execinfo.c)
    IF(NOT HAS_EXECINFO)
        CHECK_INCLUDE_FILE(execinfo.h HAVE_EXECINFO_H)
        FIND_LIBRARY(LIB_EXECINFO execinfo)
        IF(NOT HAVE_EXECINFO_H OR NOT LIB_EXECINFO)
            MESSAGE(FATAL_ERROR
                    "Could not find libexecinfo.so, which is needed for backtrace generation.")
        ENDIF(NOT HAVE_EXECINFO_H OR NOT LIB_EXECINFO)
    ENDIF(NOT HAS_EXECINFO)

    CHECK_INCLUDE_FILE(bfd.h HAVE_BFD_H)
    CHECK_INCLUDE_FILE(demangle.h HAVE_DEMANGLE_H)
    FIND_LIBRARY(LIB_BFD bfd)
    IF(NOT HAVE_BFD_H OR NOT HAVE_DEMANGLE_H OR NOT LIB_BFD)
        MESSAGE(WARNING "Without the bfd.h, demangle.h or libbfd backtraces will not contain source file name information. Install the missing SDK (e.g. binutils-dev) to enable this feature.")
    ENDIF(NOT HAVE_BFD_H OR NOT HAVE_DEMANGLE_H OR NOT LIB_BFD)
ENDIF(NOT WIN32)

FIND_PACKAGE(Qt4 COMPONENTS QtCore)
SET(HAVE_QT ${QT4_FOUND} PARENT_SCOPE)
IF(QT4_FOUND)
    INCLUDE(${QT_USE_FILE})
ENDIF(QT4_FOUND)

SET(TRACELIB_SOURCES
        core.cpp
        serializer.cpp
        output.cpp
        filter.cpp
        configuration.cpp
        backtrace.cpp
        errorlog.cpp
        variabledumping.cpp
        filemodificationmonitor.cpp
        networkoutput.cpp
        shutdownnotifier.cpp
        tracelib.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/wildcmp/wildcmp.c
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxml.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxmlerror.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxmlparser.cpp)

SET(TRACELIB_QTSUPPORT_SOURCES
        variabledumping_qt.cpp)

IF(WIN32)
    ADD_DEFINITIONS(-DSTACKWALKER_NO_GETUSERNAME)
    SET(TRACELIB_SOURCES
            ${TRACELIB_SOURCES}
            backtrace_win.cpp
            configuration_win.cpp
            crashhandler_win.cpp
            getcurrentthreadid_win.cpp
            filemodificationmonitor_win.cpp
            mutex_win.cpp
            ${PROJECT_SOURCE_DIR}/3rdparty/stackwalker/StackWalker.cpp)
ELSE(WIN32)
    SET(TRACELIB_SOURCES
            ${TRACELIB_SOURCES}
            backtrace_unix.cpp
            configuration_unix.cpp
            crashhandler_unix.cpp
            getcurrentthreadid_unix.cpp
            filemodificationmonitor_unix.cpp
            mutex_unix.cpp)
ENDIF(WIN32)

IF(WIN32)
    SET_SOURCE_FILES_PROPERTIES(filter.cpp PROPERTIES COMPILE_FLAGS /I${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10)
ELSE(WIN32)
    SET_SOURCE_FILES_PROPERTIES(filter.cpp PROPERTIES COMPILE_FLAGS -I${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10)
ENDIF(WIN32)

ADD_LIBRARY(tracelib STATIC ${TRACELIB_SOURCES})
TARGET_LINK_LIBRARIES(tracelib pcre pcrecpp)
TARGET_LINK_LIBRARIES(tracelib LINK_INTERFACE_LIBRARIES pcre pcrecpp)

IF(WIN32)
    TARGET_LINK_LIBRARIES(tracelib ws2_32.lib)
    TARGET_LINK_LIBRARIES(tracelib LINK_INTERFACE_LIBRARIES ws2_32.lib)
ELSE(WIN32)
    IF(LIB_BFD)
        TARGET_LINK_LIBRARIES(tracelib ${LIB_BFD})
        TARGET_LINK_LIBRARIES(tracelib LINK_INTERFACE_LIBRARIES ${LIB_BFD})
    ENDIF(LIB_BFD)
    IF(LIB_EXECINFO)
        TARGET_LINK_LIBRARIES(tracelib ${LIB_EXECINFO})
        TARGET_LINK_LIBRARIES(tracelib LINK_INTERFACE_LIBRARIES ${LIB_EXECINFO})
    ENDIF(LIB_EXECINFO)
ENDIF(WIN32)

IF(QT4_FOUND)
    ADD_LIBRARY(tracelib_qtsupport STATIC ${TRACELIB_QTSUPPORT_SOURCES})
    TARGET_LINK_LIBRARIES(tracelib_qtsupport ${QT_LIBRARIES})
ENDIF(QT4_FOUND)

# Installation
INSTALL(TARGETS tracelib EXPORT tracelibTargets
                         RUNTIME DESTINATION bin
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib COMPONENT devel)

IF(QT4_FOUND)
    INSTALL(TARGETS tracelib_qtsupport EXPORT tracelib_qtsupportTargets
                                       RUNTIME DESTINATION bin
                                       LIBRARY DESTINATION lib
                                       ARCHIVE DESTINATION lib COMPONENT devel)
ENDIF(QT4_FOUND)

INSTALL(FILES 
        tracelib.h 
        tracelib_config.h 
        variabledumping.h 
        variabletypes.def
        tracepointtypes.def
        DESTINATION include/tracelib COMPONENT devel)

