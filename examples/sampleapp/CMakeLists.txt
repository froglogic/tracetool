CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(sampleapp)
FIND_PACKAGE(tracelib 1.1.1 REQUIRED)

INCLUDE_DIRECTORIES(${TRACELIB_INCLUDE_DIR})

ADD_EXECUTABLE(sampleapp sampleapp.cpp person.cpp acme.cpp)
TARGET_LINK_LIBRARIES(sampleapp ${TRACELIB_LIBRARIES})
IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    SET_TARGET_PROPERTIES(sampleapp PROPERTIES COMPILE_FLAGS "/FC")
ENDIF(MSVC)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/tracelib.xml
        ${CMAKE_CURRENT_BINARY_DIR}/tracelib.xml
        COPYONLY)

# No installation as this is just a sample app
# INSTALL(TARGETS sampleapp RUNTIME DESTINATION examples/sampleapp
#                           LIBRARY DESTINATION lib
#                           ARCHIVE DESTINATION lib COMPONENT devel)
# INSTALL(FILES tracelib.xml DESTINATION examples/sampleapp)
