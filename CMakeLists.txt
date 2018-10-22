CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
message(STATUS "Using CMake ${CMAKE_VERSION}")
PROJECT(tracelib)

# call in tests/CMakeFiles.txt should suffice but the tests target is
# non-functional more often than working unless this is turned on
# earlier.
ENABLE_TESTING()

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${PROJECT_SOURCE_DIR}/cmake/modules)
OPTION(HOOKLIB_ONLY "Build only the hook library" OFF)
OPTION(ENABLE_INSTALL_RPATH "Enable setting of CMAKE_INSTALL_RPATH_USE_LINK_PATH, useful for local installations, but problematic when creating packages as buildsystem paths may leak into packages. Defaults to ON" ON)
OPTION(BUNDLE_QT "Bundle the Qt libraries/plugins in the installation folder" OFF)


if(NOT HOOKLIB_ONLY)
    FIND_PACKAGE(Qt5 COMPONENTS Core Network Xml Sql LinguistTools REQUIRED)
    # Used in the cmake-config.h.in file
    set(HAVE_QT 1)
else()
    set(HAVE_QT 0)
endif()

IF(WIN32)
    SET(execext ".exe")
elseif(APPLE)
    set(execext ".app")
ENDIF()

IF(ENABLE_INSTALL_RPATH)
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF()

INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

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
    ADD_SUBDIRECTORY(xml2trace)
    ADD_SUBDIRECTORY(tests)
    ADD_SUBDIRECTORY(examples/sampleapp)
    ADD_SUBDIRECTORY(examples/addressbook)
    ADD_SUBDIRECTORY(examples)
endif()
set(PCRE_BUILD_TESTS OFF CACHE BOOL "Build the tests")
set(PCRE_BUILD_PCREGREP OFF)
ADD_SUBDIRECTORY(3rdparty/pcre-8.10)

SET(TRACELIB_VERSION_MAJOR 3)
SET(TRACELIB_VERSION_MINOR 0)
SET(TRACELIB_VERSION_PATCH 1)

CONFIGURE_FILE(config-cmake.h.in
    ${PROJECT_BINARY_DIR}/config.h
    @ONLY)

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    CONFIGURE_FILE(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)
    ADD_CUSTOM_TARGET(api-docs ALL ${DOXYGEN_EXECUTABLE} \"${PROJECT_BINARY_DIR}/Doxyfile\" COMMENT "Generating API documentation")

    # Installation of api docs
    INSTALL(DIRECTORY ${PROJECT_BINARY_DIR}/doc/html DESTINATION share/doc/tracelib COMPONENT documentation)
ENDIF(DOXYGEN_FOUND)

# Generate CMake config-Module
SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/tracelibConfigVersion.cmake.in ${PROJECT_BINARY_DIR}/tracelibConfigVersion.cmake @ONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/tracelibConfig.cmake.in ${PROJECT_BINARY_DIR}/tracelibConfig.cmake @ONLY)
INSTALL(FILES ${PROJECT_BINARY_DIR}/tracelibConfig.cmake
              ${PROJECT_BINARY_DIR}/tracelibConfigVersion.cmake
              DESTINATION lib/cmake/tracelib COMPONENT library)
INSTALL(EXPORT tracelibTargets
              DESTINATION lib/cmake/tracelib COMPONENT library
              NAMESPACE tracelibImport__
              FILE tracelibTargets.cmake)

# Add cppcheck target
FIND_PROGRAM(CPPCHECK_EXE cppcheck)
IF(CPPCHECK_EXE)
    SET(cppcheck_include_paths -Ihooklib -Iserver -Igui)
    SET(cppcheck_ignore_paths -i3rdparty)
    SET(cppcheck_paths hooklib server gui tests examples convertdb trace2xml xml2trace)
    ADD_CUSTOM_TARGET(cppcheck
        COMMAND ${CPPCHECK_EXE} --enable=all --quiet --xml --xml-version=2
                ${cppcheck_include_paths} ${cppcheck_ignore_paths}
                ${cppcheck_paths}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Runs cppcheck on the sources")
ENDIF()

## Package Creation Setup
# See http://www.paraview.org/Wiki/CMake:CPackConfiguration for documentation of variables
SET(CPACK_PACKAGE_VERSION_MAJOR ${TRACELIB_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${TRACELIB_VERSION_MINOR})
if(HOOKLIB_ONLY)
    set(CPACK_PACKAGE_NAME "tracelib-hooklib")
endif()
set(PACKAGE_SYSTEM_NAME_SUFFIX "")
if(CMAKE_CROSSCOMPILING)
    set(PACKAGE_SYSTEM_NAME_SUFFIX "-${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
# At least on my linux system CMake does not append -i686 to system name and even if it would
# the value would be wrong. So use sizeof(void*) to add 32/64 to the package name
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(PACKAGE_SYSTEM_NAME_SUFFIX "-x86")
    else()
        set(PACKAGE_SYSTEM_NAME_SUFFIX "-x64")
    endif()
endif()

set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY 1) # Make sure to get top-level directory even when using component packages
set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}${PACKAGE_SYSTEM_NAME_SUFFIX}")
SET(CPACK_PACKAGE_VERSION_PATCH ${TRACELIB_VERSION_PATCH})
SET(CPACK_PACKAGE_VENDOR "froglogic GmbH")
SET(CPACK_PACKAGE_DESCRIPTION "Trace library, documentation and runtime applications")
SET(CPACK_SOURCE_IGNORE_FILES "/build/;/.git/;.gitignore")
# Setup components stuff
# Enable all archive-package-generators to use componentized install
# the default is to do all-in-one install packages since thats the backwards-compatible behaviour
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL applications library examples documentation)
set(CPACK_COMPONENT_LIBRARY_GROUP "Library")
set(CPACK_COMPONENT_EXAMPLES_GROUP "Library")
set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION "Components needed to use trace library in an application and examples")
set(CPACK_COMPONENT_APPLICATIONS_GROUP "Tools")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Applications")
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "Applications to receive trace output, analyze it and convert between formats")
set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Examples")
set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Example applications showing the usage of the Trace library")
set(CPACK_COMPONENT_LIBRARY_DISPLAY_NAME "Hook Library")
set(CPACK_COMPONENT_LIBRARY_DESCRIPTION "Trace library and headers to link into the application")
set(CPACK_COMPONENT_DOCUMENTATION_GROUP "Documentation")
set(CPACK_COMPONENT_GROUP_DOCUMENTATION_DESCRIPTION "Documentation for the Trace library")
set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "Documentation for the Trace library")
IF(WIN32)
    SET(CPACK_GENERATOR "ZIP")
    SET(CPACK_SOURCE_GENERATOR "ZIP")
ELSEIF(APPLE)
    SET(CPACK_GENERATOR "ZIP")
    SET(CPACK_SOURCE_GENERATOR "TGZ")
ELSE(WIN32)
    SET(CPACK_GENERATOR "TGZ")
    SET(CPACK_SOURCE_GENERATOR "TGZ")
ENDIF(WIN32)
INCLUDE(CPack)
