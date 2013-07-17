CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(tracelib)

# call in tests/CMakeFiles.txt should suffice but the tests target is
# non-functional more often than working unless this is turned on
# earlier.
ENABLE_TESTING()

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${PROJECT_SOURCE_DIR}/cmake/modules)
OPTION(HOOKLIB_ONLY "Build only the hook library" OFF)

if(NOT HOOKLIB_ONLY)
    FIND_PACKAGE(Qt4 4.6 COMPONENTS QtCore QtNetwork QtXml QtSql QtGui REQUIRED)
    INCLUDE(${QT_USE_FILE})
    INCLUDE(DeployQt4)
    # Used in the cmake-config.h.in file
    set(HAVE_QT 1)
else()
    set(HAVE_QT 0)
endif()
IF(WIN32)
    SET(execext ".exe")
ENDIF()

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})

IF(CMAKE_COMPILER_IS_GNUCC)
    # TODO: consider adding -Wconversion
    ADD_DEFINITIONS(-Wall -pedantic -Wwrite-strings -Wcast-align
                    -Wchar-subscripts -Wno-long-long)
ENDIF(CMAKE_COMPILER_IS_GNUCC)

ADD_SUBDIRECTORY(hooklib)
if(NOT HOOKLIB_ONLY)
    ADD_SUBDIRECTORY(server)
    ADD_SUBDIRECTORY(gui)
    ADD_SUBDIRECTORY(convertdb)
    ADD_SUBDIRECTORY(trace2xml)
    ADD_SUBDIRECTORY(tests)
    ADD_SUBDIRECTORY(examples/sampleapp)
    ADD_SUBDIRECTORY(examples/addressbook)
    ADD_SUBDIRECTORY(examples)
endif()
ADD_SUBDIRECTORY(3rdparty/pcre-8.10)

SET(TRACELIB_VERSION_MAJOR 1)
SET(TRACELIB_VERSION_MINOR 8)
SET(TRACELIB_VERSION_PATCH 4)

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

## Package Creation Setup
# See http://www.paraview.org/Wiki/CMake:CPackConfiguration for documentation of variables
SET(CPACK_PACKAGE_VERSION_MAJOR ${TRACELIB_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${TRACELIB_VERSION_MINOR})
if(HOOKLIB_ONLY)
    set(CPACK_PACKAGE_NAME "tracelib-hooklib")
endif()
# This is ugly, but the only way to get a msvc-version added to the package name
# CMake does not set the CMAKE_CXX_COMPILER_VERSION variable (at least in 2.8.7)
SET(compilerSuffix "")
IF(WIN32)
    IF(MSVC60)
        SET(compilerSuffix "-MSVC6")
    ELSEIF(MSVC70 OR MSVC71)
        SET(compilerSuffix "-MSVC7")
    ELSEIF(MSVC80)
        SET(compilerSuffix "-MSVC8")
    ELSEIF(MSVC90)
        SET(compilerSuffix "-MSVC9")
    ELSEIF(MSVC10)
        SET(compilerSuffix "-MSVC10")
    ELSEIF(MSVC11)
        SET(compilerSuffix "-MSVC11")
    ELSEIF(COMPILER_CXX_IS_GNU)
        SET(compilerSuffix "-gcc")
    ELSE()
        SET(compilerSuffix "-Unknown")
    ENDIF()
endif()
SET(CPACK_PACKAGE_VERSION_PATCH ${TRACELIB_VERSION_PATCH}${compilerSuffix})
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
