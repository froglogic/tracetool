CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(tracelib)

# call in tests/CMakeFiles.txt should suffice but the tests target is
# non-functional more often than working unless this is turned on
# earlier.
ENABLE_TESTING()

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${PROJECT_SOURCE_DIR}/cmake/modules)

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

IF(CMAKE_COMPILER_IS_GNUCC)
    # TODO: consider adding -Wconversion
    ADD_DEFINITIONS(-Wall -pedantic -Wwrite-strings -Wcast-align
                    -Wchar-subscripts -Wno-long-long)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

ADD_SUBDIRECTORY(hooklib)
ADD_SUBDIRECTORY(server)
ADD_SUBDIRECTORY(gui)
ADD_SUBDIRECTORY(convertdb)
ADD_SUBDIRECTORY(trace2xml)
ADD_SUBDIRECTORY(3rdparty/pcre-8.10)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(examples/sampleapp)
ADD_SUBDIRECTORY(examples/addressbook)
ADD_SUBDIRECTORY(examples)

SET(TRACELIB_VERSION_MAJOR 1)
SET(TRACELIB_VERSION_MINOR 4)
SET(TRACELIB_VERSION_PATCH 2)

CONFIGURE_FILE(config-cmake.h.in
    ${PROJECT_BINARY_DIR}/config.h
    @ONLY)

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    CONFIGURE_FILE(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)
    ADD_CUSTOM_TARGET(api-docs ALL ${DOXYGEN_EXECUTABLE} \"${PROJECT_BINARY_DIR}/Doxyfile\" COMMENT "Generating API documentation")

    # Installation of api docs
    INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html DESTINATION share/doc/tracelib COMPONENT devel)
ENDIF(DOXYGEN_FOUND)

# Generate CMake config-Module
SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/tracelibConfigVersion.cmake.in ${PROJECT_BINARY_DIR}/tracelibConfigVersion.cmake @ONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/tracelibConfig.cmake.in ${PROJECT_BINARY_DIR}/tracelibConfig.cmake @ONLY)
INSTALL(FILES ${PROJECT_BINARY_DIR}/tracelibConfig.cmake
              ${PROJECT_BINARY_DIR}/tracelibConfigVersion.cmake
              DESTINATION lib/cmake/tracelib COMPONENT devel)
INSTALL(EXPORT tracelibTargets
              DESTINATION lib/cmake/tracelib COMPONENT devel
              NAMESPACE tracelibImport__
              FILE tracelibTargets.cmake)

IF(CMAKE_BUILD_TYPE MATCHES "Rel")
INSTALL(FILES
        ${QT_BINARY_DIR}/QtCore4.dll
        ${QT_BINARY_DIR}/QtGui4.dll
        ${QT_BINARY_DIR}/QtNetwork4.dll
        ${QT_BINARY_DIR}/QtSql4.dll
        ${QT_BINARY_DIR}/QtXml4.dll
        DESTINATION bin)
INSTALL(FILES
        ${QT_PLUGINS_DIR}/sqldrivers/qsqlite4.dll
        DESTINATION bin/sqldrivers)
ELSE(CMAKE_BUILD_TYPE MATCHES "Rel")
INSTALL(FILES
        ${QT_BINARY_DIR}/QtCored4.dll
        ${QT_BINARY_DIR}/QtGuid4.dll
        ${QT_BINARY_DIR}/QtNetworkd4.dll
        ${QT_BINARY_DIR}/QtSqld4.dll
        ${QT_BINARY_DIR}/QtXmld4.dll
        DESTINATION bin)
INSTALL(FILES
        ${QT_PLUGINS_DIR}/sqldrivers/qsqlited4.dll
        DESTINATION bin/sqldrivers)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Rel")

## Package Creation Setup
# See http://www.paraview.org/Wiki/CMake:CPackConfiguration for documentation of variables
SET(CPACK_PACKAGE_VERSION_MAJOR ${TRACELIB_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${TRACELIB_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${TRACELIB_VERSION_PATCH})
SET(CPACK_PACKAGE_VENDOR "froglogic GmbH")
SET(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;.gitignore")
IF(WIN32)
    SET(CPACK_GENERATOR "ZIP")
    SET(CPACK_SOURCE_GENERATOR "ZIP")
ELSEIF(APPLE)
    SET(CPACK_GENERATOR "Bundle")
    SET(CPACK_SOURCE_GENERATOR "TGZ")
ELSE(WIN32)
    SET(CPACK_GENERATOR "TGZ")
    SET(CPACK_SOURCE_GENERATOR "TGZ")
ENDIF(WIN32)
INCLUDE(CPack)
