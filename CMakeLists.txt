CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(tracelib)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF(MSVC)

ADD_DEFINITIONS(-DPCRE_STATIC -DTIXML_USE_STL)
ADD_SUBDIRECTORY(3rdparty/pcre-8.10)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(examples/sampleapp)
ADD_SUBDIRECTORY(examples/tracegen)

SET(TRACELIB_SOURCES
        core.cpp
        serializer.cpp
        output.cpp
        filter.cpp
        configuration.cpp
        backtrace.cpp
        errorlog.cpp
        variabledumping.cpp
        3rdparty/wildcmp/wildcmp.c
        3rdparty/tinyxml/tinyxml.cpp
        3rdparty/tinyxml/tinyxmlerror.cpp
        3rdparty/tinyxml/tinyxmlparser.cpp)

IF(WIN32)
    SET(TRACELIB_SOURCES
            ${TRACELIB_SOURCES}
            backtrace_win.cpp
            configuration_win.cpp
            networkoutput_win.cpp
            crashhandler_win.cpp
            getcurrentthreadid_win.cpp
            3rdparty/stackwalker/StackWalker.cpp)
ENDIF(WIN32)

SET_SOURCE_FILES_PROPERTIES(filter.cpp PROPERTIES COMPILE_FLAGS /I${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10)

ADD_LIBRARY(tracelib STATIC ${TRACELIB_SOURCES})
TARGET_LINK_LIBRARIES(tracelib pcre pcrecpp)
IF(WIN32)
    TARGET_LINK_LIBRARIES(tracelib ws2_32.lib)
ENDIF(WIN32)

CONFIGURE_FILE(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)
INCLUDE(${CMAKE_ROOT}/Modules/FindDoxygen.cmake)
ADD_CUSTOM_TARGET(api-docs ${DOXYGEN} \"${PROJECT_BINARY_DIR}/Doxyfile\" COMMENT "Generating API documentation")

