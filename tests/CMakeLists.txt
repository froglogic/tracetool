INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/hooklib
        ${PROJECT_SOURCE_DIR}/3rdparty/pcre-8.10
)

ADD_DEFINITIONS(-DTRACELIB_EXPORT= -DPCRE_STATIC -D_CRT_SECURE_NO_WARNINGS)

# PCRE is linked with /MT so all executables linking to directly also need to use MT so do this for the whole dir for now.
IF(MSVC)
    #on windows we link the C runtime statically so that our trace library
    #does not interfere with the users c runtime
    string(TOUPPER ${CMAKE_BUILD_TYPE} UC_BUILD_TYPE)
    string(REPLACE "/MD" "/MT" "CMAKE_C_FLAGS_${UC_BUILD_TYPE}" "${CMAKE_C_FLAGS_${UC_BUILD_TYPE}}")
    string(REPLACE "/MD" "/MT" "CMAKE_CXX_FLAGS_${UC_BUILD_TYPE}" "${CMAKE_CXX_FLAGS_${UC_BUILD_TYPE}}")
ENDIF(MSVC)
ADD_EXECUTABLE(test_filter
        test_filter.cpp
        ../hooklib/filter.cpp
        ../3rdparty/wildcmp/wildcmp.c)
TARGET_LINK_LIBRARIES(test_filter pcre pcrecpp)

IF(WIN32)
    ADD_EXECUTABLE(test_info
            test_info.cpp
            ../hooklib/getcurrentthreadid_win.cpp
            ../hooklib/timehelper.cpp)
ELSE(WIN32)
    ADD_EXECUTABLE(test_info
            test_info.cpp
            ../hooklib/getcurrentthreadid_unix.cpp
            ../hooklib/timehelper.cpp)
ENDIF(WIN32)

IF(WIN32)
    ADD_EXECUTABLE(test_processname
            test_processname.cpp
            ../hooklib/getcurrentthreadid_win.cpp
            ../hooklib/timehelper.cpp
            ../hooklib/configuration_win.cpp)
ELSE(WIN32)
    ADD_EXECUTABLE(test_processname
            test_processname.cpp
            ../hooklib/getcurrentthreadid_unix.cpp
            ../hooklib/timehelper.cpp
            ../hooklib/configuration_unix.cpp)
ENDIF(WIN32)

IF(NOT WIN32 AND NOT APPLE)
    find_package(Threads REQUIRED)
    if( ${CMAKE_USE_PTHREADS_INIT} )
        ADD_EXECUTABLE(test_eventthreadunix test_eventthreadunix.cpp)
        TARGET_LINK_LIBRARIES(test_eventthreadunix tracelib ${CMAKE_THREAD_LIBS_INIT})
    endif()
ENDIF()

FIND_PACKAGE(Qt5 COMPONENTS Gui Core Sql Network Xml Sql REQUIRED)
QT5_WRAP_CPP(TESTSESSION_MOC_SOURCES ../gui/columnsinfo.h)
ADD_EXECUTABLE(test_session test_session.cpp
                            ../gui/columnsinfo.cpp
                            ${TESTSESSION_MOC_SOURCES})
TARGET_LINK_LIBRARIES(test_session Qt5::Core)

QT5_WRAP_CPP(TESTGUICONF_MOC_SOURCES ../gui/configuration.h)
ADD_EXECUTABLE(test_guiconf test_guiconf.cpp
                            ../gui/configuration.cpp
                            ${TESTGUICONF_MOC_SOURCES})
TARGET_LINK_LIBRARIES(test_guiconf Qt5::Core)

ENABLE_TESTING()
ADD_TEST(NAME test_filter COMMAND test_filter)
ADD_TEST(NAME test_processid COMMAND test_info --processid)
ADD_TEST(NAME test_threadid COMMAND test_info --threadid)
ADD_TEST(NAME test_starttime COMMAND test_info --starttime)
ADD_TEST(NAME test_processname COMMAND test_processname)
ADD_TEST(NAME test_columninfo COMMAND test_session --columns)
ADD_TEST(NAME test_guiconf COMMAND test_guiconf ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(test_filter
    test_processid
    test_threadid
    test_starttime
    test_processname
    test_columninfo
    test_guiconf 
    PROPERTIES TIMEOUT 60)
