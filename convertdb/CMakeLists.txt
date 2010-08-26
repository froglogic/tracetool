SET(CONVERTDB_SOURCES
        main.cpp
        getopt.cpp
        ../server/database.cpp
        ../server/server.cpp)

SET(CONVERTDB_MOCABLES
  ../server/server.h)

FIND_PACKAGE(Qt4 4.5.0 COMPONENTS QtNetwork QtXml QtSql QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(CONVERTDB_MOC_SOURCES ${CONVERTDB_MOCABLES})


ADD_EXECUTABLE(convertdb ${CONVERTDB_SOURCES}
                         ${CONVERTDB_MOC_SOURCES})
TARGET_LINK_LIBRARIES(convertdb ${QT_LIBRARIES})

