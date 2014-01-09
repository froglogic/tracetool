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

IF(NOT WIN32)
    find_package(Threads REQUIRED)
    if( ${CMAKE_USE_PTHREADS_INIT} )
        ADD_EXECUTABLE(test_eventthreadunix test_eventthreadunix.cpp)
        TARGET_LINK_LIBRARIES(test_eventthreadunix tracelib ${CMAKE_THREAD_LIBS_INIT})
    endif()
ENDIF(NOT WIN32)

FIND_PACKAGE(Qt4 4.6 COMPONENTS QtGui QtCore QtSql QtNetwork QtXml QtSql REQUIRED)
INCLUDE(${QT_USE_FILE})
QT4_WRAP_CPP(TESTSESSION_MOC_SOURCES ../gui/columnsinfo.h)
ADD_EXECUTABLE(test_session test_session.cpp
                            ../gui/columnsinfo.cpp
                            ${TESTSESSION_MOC_SOURCES})
TARGET_LINK_LIBRARIES(test_session ${QT_LIBRARIES})

QT4_WRAP_CPP(TESTGUICONF_MOC_SOURCES ../gui/configuration.h)
ADD_EXECUTABLE(test_guiconf test_guiconf.cpp
                            ../gui/configuration.cpp
                            ${TESTGUICONF_MOC_SOURCES})
TARGET_LINK_LIBRARIES(test_guiconf ${QT_LIBRARIES})

ENABLE_TESTING()
ADD_TEST(NAME test_filter COMMAND test_filter)
ADD_TEST(NAME test_processid COMMAND test_info --processid)
ADD_TEST(NAME test_threadid COMMAND test_info --threadid)
ADD_TEST(NAME test_starttime COMMAND test_info --starttime)
ADD_TEST(NAME test_processname COMMAND test_processname)
ADD_TEST(NAME test_columninfo COMMAND test_session --columns)
ADD_TEST(NAME test_guiconf COMMAND test_guiconf ${CMAKE_CURRENT_SOURCE_DIR})
