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
    IF(NOT HAVE_BFD_H OR NOT HAVE_DEMANGLE_H)
        MESSAGE(WARNING "Without the bfd.h and demangle.h file backtraces will not contain source file name information. Install the missing SDK (e.g. binutils-dev) to enable this feature.")
    ENDIF(NOT HAVE_BFD_H OR NOT HAVE_DEMANGLE_H)
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
        ${PROJECT_SOURCE_DIR}/3rdparty/wildcmp/wildcmp.c
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxml.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxmlerror.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxmlparser.cpp)

IF(WIN32)
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
IF(QT4_FOUND)
    TARGET_LINK_LIBRARIES(tracelib ${QT_LIBRARIES})
ENDIF(QT4_FOUND)

IF(WIN32)
    TARGET_LINK_LIBRARIES(tracelib ws2_32.lib)
ELSE(WIN32)
    IF(HAVE_BFD_H)
        TARGET_LINK_LIBRARIES(tracelib bfd)
    ENDIF(HAVE_BFD_H)
    IF(LIB_EXECINFO)
        TARGET_LINK_LIBRARIES(tracelib ${LIB_EXECINFO})
    ENDIF(LIB_EXECINFO)
ENDIF(WIN32)

