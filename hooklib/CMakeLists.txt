INCLUDE(CheckIncludeFile)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    #on windows we link the C runtime statically so that our trace library
    #does not interfere with the users c runtime
    string(TOUPPER ${CMAKE_BUILD_TYPE} UC_BUILD_TYPE)
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_${UC_BUILD_TYPE} ${CMAKE_CXX_FLAGS_${UC_BUILD_TYPE}})
    string(REPLACE "/MD" "/MT" "CMAKE_C_FLAGS_${UC_BUILD_TYPE}" "${CMAKE_C_FLAGS_${UC_BUILD_TYPE}}")
ENDIF(MSVC)

IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    SET(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES} /usr/local/include")
    INCLUDE_DIRECTORIES("/usr/local/include")
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

ADD_DEFINITIONS(-DPCRE_STATIC -DTIXML_USE_STL -DTRACELIB_MAKEDLL)

IF(NOT WIN32)
    CHECK_INCLUDE_FILE(execinfo.h HAVE_EXECINFO_H)
    # If we cannot use execinfo.h and backtrace() out of the box, try to find
    # libexecinfo which provides (roughly) the same functionality.
    TRY_COMPILE(HAS_EXECINFO
                ${PROJECT_BINARY_DIR}/config-tests
                ${PROJECT_SOURCE_DIR}/config-tests/execinfo.c)
    IF(NOT HAS_EXECINFO)
        FIND_LIBRARY(LIB_EXECINFO execinfo)
        IF(NOT HAVE_EXECINFO_H OR NOT LIB_EXECINFO)
            MESSAGE(WARNING
                    "Could not find libexecinfo.so, which is needed for backtrace generation.")
        ENDIF(NOT HAVE_EXECINFO_H OR NOT LIB_EXECINFO)
    ENDIF(NOT HAS_EXECINFO)

    CHECK_INCLUDE_FILE(sys/inotify.h HAVE_INOTIFY_H)
    CHECK_INCLUDE_FILE(bfd.h HAVE_BFD_H)
    CHECK_INCLUDE_FILE(demangle.h HAVE_DEMANGLE_H)
    # In newer Debian's demangle.h and the libiberty library are separated into
    # their own package and have a dedicated include dir
    if(NOT HAVE_DEMANGLE_H)
        find_path(LIBIBERTY_INCLUDE_DIR NAMES demangle.h PATH_SUFFIXES libiberty)
        find_library(LIBIBERTY_LIBRARIES iberty)
    endif()
    # Switch preference of library suffixes to ensure we look for the static bfd
    # first and prefer that. Unfortunately the bfd library is installed with its
    # version in the filename so linking it means having to have the same bfd version
    # on all target systems
    set(origSuffixes "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".so")
    FIND_LIBRARY(LIB_BFD bfd)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${origSuffixes}")
    IF(NOT HAVE_BFD_H OR NOT (HAVE_DEMANGLE_H OR (LIBIBERTY_INCLUDE_DIR AND LIBIBERTY_LIBRARIES) ) OR NOT LIB_BFD)
        if( NOT HAVE_BFD_H )
            message(WARNING "bfd.h could not be found")
        endif()
        if( NOT ( HAVE_DEMANGLE_H OR (LIBIBERTY_INCLUDE_DIR AND LIBIBERTY_LIBRARIES) ) )
            if( NOT LIBIBERTY_INCLUDE_DIR )
                message( WARNING "Could not find demangle.h in standard include directories or in a libiberty/ subdirectory" )
            elseif( NOT LIBIBERTY_INCLUDE_DIR )
                message( WARNING "Could not find iberty library, this is usually libiberty.a" )
            else()
                message( WARNING "Could not find demangle.h on the system" )
            endif()
        endif()
        if( NOT LIB_BFD )
            message( WARNING "Failed to find libbfd.a" )
        endif()
        message(WARNING "Without the bfd.h, demangle.h or libbfd backtraces will not contain source file name information. Install the missing SDK (e.g. binutils-dev and maybe libiberty-dev) to enable this feature. (see above for the actual files missing)")
    ENDIF()
    find_package(Threads)
    if(NOT CMAKE_USE_PTHREADS_INIT )
        message(WARNING "No pthreads found, linking will likely fail.")
    endif()
ENDIF(NOT WIN32)

SET(TRACELIB_PUBLIC_HEADERS
        dlldefs.h
        tracelib.h
        tracepoint.h
        tracelib_config.h
        variabledumping.h
        tracelib_qt.h
        variabledumping_qt.h)

SET(TRACELIB_SOURCES
        trace.cpp
        serializer.cpp
        output.cpp
        filter.cpp
        configuration.cpp
        backtrace.cpp
        log.cpp
        variabledumping.cpp
        filemodificationmonitor.cpp
        shutdownnotifier.cpp
        tracelib.cpp
        timehelper.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/wildcmp/wildcmp.c
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxml.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxmlerror.cpp
        ${PROJECT_SOURCE_DIR}/3rdparty/tinyxml/tinyxmlparser.cpp)

IF(WIN32)
    ADD_DEFINITIONS(-DSTACKWALKER_NO_GETUSERNAME)
    SET(TRACELIB_SOURCES
            ${TRACELIB_SOURCES}
            backtrace_win.cpp
            configuration_win.cpp
            crashhandler_win.cpp
            getcurrentthreadid_win.cpp
            filemodificationmonitor_win.cpp
            networkoutput.cpp
            mutex_win.cpp
            ${PROJECT_SOURCE_DIR}/3rdparty/stackwalker/StackWalker.cpp)
ELSE(WIN32)
    SET(TRACELIB_SOURCES
            ${TRACELIB_SOURCES}
            backtrace_unix.cpp
            configuration_unix.cpp
            eventthread_unix.cpp
            crashhandler_unix.cpp
            getcurrentthreadid_unix.cpp
            filemodificationmonitor_unix.cpp
            networkoutput_unix.cpp
            mutex_unix.cpp)
ENDIF(WIN32)

IF(WIN32)
    SET_SOURCE_FILES_PROPERTIES(filter.cpp PROPERTIES COMPILE_FLAGS /I${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10)
ELSE(WIN32)
    SET_SOURCE_FILES_PROPERTIES(filter.cpp PROPERTIES COMPILE_FLAGS -I${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10)
ENDIF(WIN32)

# Assemble list of libraries to link tracelib against
SET(TRACELIB_LIBRARIES pcre pcrecpp)
IF(WIN32)
    SET(TRACELIB_LIBRARIES ${TRACELIB_LIBRARIES} ws2_32.lib shell32.lib)
ELSE(WIN32)
    IF(LIB_BFD)
        SET(TRACELIB_LIBRARIES ${TRACELIB_LIBRARIES} ${LIB_BFD})
    ENDIF(LIB_BFD)
    if(LIBIBERTY_LIBRARIES)
        set(TRACELIB_LIBRARIES ${TRACELIB_LIBRARIES} ${LIBIBERTY_LIBRARIES})
    endif()
    IF(LIB_EXECINFO)
        SET(TRACELIB_LIBRARIES ${TRACELIB_LIBRARIES} ${LIB_EXECINFO})
    ENDIF(LIB_EXECINFO)
    SET(TRACELIB_LIBRARIES ${TRACELIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )
ENDIF(WIN32)

if(LIBIBERTY_INCLUDE_DIR)
    include_directories(${LIBIBERTY_INCLUDE_DIR})
endif()

# # Export whitelist-separated string with public headers which is used by
# # Doxygen
# FOREACH(ARG ${TRACELIB_PUBLIC_HEADERS})
#     SET(TRACELIB_PUBLIC_HEADERS_WS "${TRACELIB_PUBLIC_HEADERS_WS} ${ARG}")
# ENDFOREACH(ARG ${TRACELIB_PUBLIC_HEADERS})
# SET(TRACELIB_PUBLIC_HEADERS_WS "${TRACELIB_PUBLIC_HEADERS_WS}" PARENT_SCOPE)

ADD_LIBRARY(tracelib SHARED ${TRACELIB_SOURCES})

# Allow to co-install 32 and 64bit tracelib libraries in the same directory
# if the cmake-invocation has a corresponding flag set.
if( ARCH_LIB_SUFFIX )
    message(STATUS "Change output name")
    set_target_properties(tracelib PROPERTIES OUTPUT_NAME tracelib${ARCH_LIB_SUFFIX})
endif()

TARGET_LINK_LIBRARIES(tracelib LINK_PRIVATE ${TRACELIB_LIBRARIES})
IF(WIN32)
    SET_TARGET_PROPERTIES(tracelib PROPERTIES DEBUG_POSTFIX d)
ENDIF(WIN32)

# Installation
INSTALL(TARGETS tracelib EXPORT tracelibTargets
                         RUNTIME DESTINATION bin COMPONENT library
                         LIBRARY DESTINATION lib COMPONENT library
                         ARCHIVE DESTINATION lib COMPONENT library)

INSTALL(FILES ${TRACELIB_PUBLIC_HEADERS}
        variabletypes.def
        tracepointtypes.def
        DESTINATION include/tracelib COMPONENT library)
