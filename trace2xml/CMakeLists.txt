SET(TRACE2XML_SOURCES
        main.cpp
        ../convertdb/getopt.cpp
        ../server/database.cpp)

FIND_PACKAGE(Qt4 4.5.0 COMPONENTS QtSql QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

ADD_EXECUTABLE(trace2xml ${TRACE2XML_SOURCES})
TARGET_LINK_LIBRARIES(trace2xml ${QT_LIBRARIES})

