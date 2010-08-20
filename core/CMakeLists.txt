INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE(CheckIncludeFile)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF(MSVC)

ADD_DEFINITIONS(-DPCRE_STATIC -DTIXML_USE_STL)

IF(NOT WIN32)
    CHECK_INCLUDE_FILE(bfd.h HAVE_BFD_H)
ENDIF(NOT WIN32)

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
        ${PROJECT_SOURCE_DIR}/3rdparty/wildcmp/wildcmp.c
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxml.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxmlerror.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxmlparser.cpp)

IF(WIN32)
    SET(TRACELIB_SOURCES
            ${TRACELIB_SOURCES}
            backtrace_win.cpp
            configuration_win.cpp
            networkoutput_win.cpp
            crashhandler_win.cpp
            getcurrentthreadid_win.cpp
            filemodificationmonitor_win.cpp
            ${PROJECT_SOURCE_DIR}/3rdparty/stackwalker/StackWalker.cpp)
ELSE(WIN32)
    SET(TRACELIB_SOURCES
            ${TRACELIB_SOURCES}
            backtrace_unix.cpp
            configuration_unix.cpp
            networkoutput_unix.cpp
            getcurrentthreadid_unix.cpp
            filemodificationmonitor_unix.cpp)
ENDIF(WIN32)

IF(WIN32)
    SET_SOURCE_FILES_PROPERTIES(filter.cpp PROPERTIES COMPILE_FLAGS /I${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10)
ELSE(WIN32)
    SET_SOURCE_FILES_PROPERTIES(filter.cpp PROPERTIES COMPILE_FLAGS -I${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10)
ENDIF(WIN32)

ADD_LIBRARY(tracelib STATIC ${TRACELIB_SOURCES})
TARGET_LINK_LIBRARIES(tracelib pcre pcrecpp)
IF(WIN32)
    TARGET_LINK_LIBRARIES(tracelib ws2_32.lib)
ELSE(WIN32)
    IF(HAVE_BFD_H)
        TARGET_LINK_LIBRARIES(tracelib bfd)
    ENDIF(HAVE_BFD_H)
ENDIF(WIN32)

