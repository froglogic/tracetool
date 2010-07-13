CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF(MSVC)

SET(TRACELIB_SOURCES
        core.cpp
        serializer.cpp
        output.cpp
        filter.cpp
        configuration.cpp
        backtrace.cpp)

IF(WIN32)
    SET(TRACELIB_SOURCES
            ${TRACELIB_SOURCES}
            winstringconv.cpp
            backtrace_win.cpp
            3rdparty/stackwalker/StackWalker.cpp)
ENDIF(WIN32)

ADD_LIBRARY(tracelib STATIC ${TRACELIB_SOURCES})
IF(WIN32)
    TARGET_LINK_LIBRARIES(tracelib ws2_32.lib)
ENDIF(WIN32)

ADD_EXECUTABLE(sampleapp sampleapp.cpp)
TARGET_LINK_LIBRARIES(sampleapp tracelib)
IF(MSVC)
    SET_TARGET_PROPERTIES(sampleapp PROPERTIES COMPILE_FLAGS "/FC")
ENDIF(MSVC)

ADD_EXECUTABLE(tracegen WIN32 tracegen.cpp)
TARGET_LINK_LIBRARIES(tracegen tracelib)
IF(MSVC)
    SET_TARGET_PROPERTIES(tracegen PROPERTIES COMPILE_FLAGS "/FC")
ENDIF(MSVC)

ADD_EXECUTABLE(traceview traceview.cpp)
IF(MSVC)
    SET_TARGET_PROPERTIES(traceview PROPERTIES COMPILE_FLAGS "/FC")
ENDIF(MSVC)
IF(WIN32)
    TARGET_LINK_LIBRARIES(traceview ws2_32.lib)
ENDIF(WIN32)

