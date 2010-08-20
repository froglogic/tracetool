CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(tracelib)

CONFIGURE_FILE(config-cmake.h.in
    ${PROJECT_BINARY_DIR}/config.h
    @ONLY)

ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(server)
ADD_SUBDIRECTORY(3rdparty/pcre-8.10)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(examples/sampleapp)
ADD_SUBDIRECTORY(examples/tracegen)

CONFIGURE_FILE(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile)
INCLUDE(${CMAKE_ROOT}/Modules/FindDoxygen.cmake)
ADD_CUSTOM_TARGET(api-docs ${DOXYGEN} \"${PROJECT_BINARY_DIR}/Doxyfile\" COMMENT "Generating API documentation")

