CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF(MSVC)

ADD_DEFINITIONS(-DHAVE_CONFIG_H -DPCRE_STATIC -DTIXML_USE_STL)
ADD_SUBDIRECTORY(3rdparty/pcre-8.10 EXCLUDE_FROM_ALL)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(examples\\sampleapp)
ADD_SUBDIRECTORY(examples\\tracegen)

CONFIGURE_FILE(3rdparty/pcre-8.10/pcre_chartables.c.dist
        ${PROJECT_BINARY_DIR}/pcre_chartables.c
        COPYONLY)

SET(PCRE_SOURCES2
        ${PROJECT_BINARY_DIR}/pcre_chartables.c
        3rdparty/pcre-8.10/pcre_compile.c
        3rdparty/pcre-8.10/pcre_config.c
        3rdparty/pcre-8.10/pcre_dfa_exec.c
        3rdparty/pcre-8.10/pcre_exec.c
        3rdparty/pcre-8.10/pcre_fullinfo.c
        3rdparty/pcre-8.10/pcre_get.c
        3rdparty/pcre-8.10/pcre_globals.c
        3rdparty/pcre-8.10/pcre_info.c
        3rdparty/pcre-8.10/pcre_newline.c
        3rdparty/pcre-8.10/pcre_maketables.c
        3rdparty/pcre-8.10/pcre_ord2utf8.c
        3rdparty/pcre-8.10/pcre_refcount.c
        3rdparty/pcre-8.10/pcre_study.c
        3rdparty/pcre-8.10/pcre_tables.c
        3rdparty/pcre-8.10/pcre_try_flipped.c
        3rdparty/pcre-8.10/pcre_ucd.c
        3rdparty/pcre-8.10/pcre_valid_utf8.c
        3rdparty/pcre-8.10/pcre_version.c
        3rdparty/pcre-8.10/pcre_xclass.c)

SET(PCRECPP_SOURCES2
        3rdparty/pcre-8.10/pcrecpp.cc
        3rdparty/pcre-8.10/pcre_scanner.cc
        3rdparty/pcre-8.10/pcre_stringpiece.cc)

SET(TINYXML_SOURCES
        3rdparty/tinyxml/tinyxml.cpp
        3rdparty/tinyxml/tinyxmlerror.cpp
        3rdparty/tinyxml/tinyxmlparser.cpp)

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
        ${TINYXML_SOURCES}
        ${PCRE_SOURCES2}
        ${PCRECPP_SOURCES2})

SET_SOURCE_FILES_PROPERTIES(${PCRE_SOURCES2} ${PCRECPP_SOURCES2} filter.cpp PROPERTIES COMPILE_FLAGS /I${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10)

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

ADD_LIBRARY(tracelib STATIC ${TRACELIB_SOURCES})
IF(WIN32)
    TARGET_LINK_LIBRARIES(tracelib ws2_32.lib)
ENDIF(WIN32)

INCLUDE(${CMAKE_ROOT}/Modules/FindDoxygen.cmake)
ADD_CUSTOM_TARGET(api-docs ${DOXYGEN} Doxyfile COMMENT "Generating API documentation")

