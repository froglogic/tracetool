SET(SERVER_SOURCES
        main.cpp
        database.cpp
        server.cpp
        ../convertdb/getopt.cpp)

SET(SERVER_MOCABLES
        server.h)

FIND_PACKAGE(Qt4 4.6 COMPONENTS QtCore QtNetwork QtXml QtSql REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(SERVER_MOC_SOURCES ${SERVER_MOCABLES})

ADD_EXECUTABLE(traced ${SERVER_SOURCES} ${SERVER_MOC_SOURCES})
TARGET_LINK_LIBRARIES(traced ${QT_LIBRARIES})

# Installation
INSTALL(TARGETS traced RUNTIME DESTINATION bin
                       ARCHIVE DESTINATION lib
                       LIBRARY DESTINATION lib COMPONENT devel)

