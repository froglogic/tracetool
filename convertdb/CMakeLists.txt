SET(CONVERTDB_SOURCES
        main.cpp
        getopt.cpp
        ../server/database.cpp)

FIND_PACKAGE(Qt4 4.5.0 COMPONENTS QtSql QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

ADD_EXECUTABLE(convertdb ${CONVERTDB_SOURCES})
TARGET_LINK_LIBRARIES(convertdb ${QT_LIBRARIES})

