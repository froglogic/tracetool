SET(SERVER_SOURCES
        main.cpp
        database.cpp
        server.cpp
        databasefeeder.cpp
        xmlcontenthandler.cpp
        ../convertdb/getopt.cpp)

SET(SERVER_MOCABLES
        server.h)
SET(SERVER_TS
        ${CMAKE_CURRENT_BINARY_DIR}/server.ts)

QT5_WRAP_CPP(SERVER_MOC_SOURCES ${SERVER_MOCABLES})
QT5_CREATE_TRANSLATION( SERVER_QM ${SERVER_SOURCES} ${SERVER_TS})

ADD_EXECUTABLE(traced MACOSX_BUNDLE ${SERVER_SOURCES} ${SERVER_MOC_SOURCES} ${SERVER_QM})
TARGET_LINK_LIBRARIES(traced Qt5::Core Qt5::Network Qt5::Sql)

# Installation
INSTALL(TARGETS traced RUNTIME DESTINATION bin COMPONENT applications
                       ARCHIVE DESTINATION lib COMPONENT applications
                       BUNDLE  DESTINATION bin COMPONENT applications
                       LIBRARY DESTINATION lib COMPONENT applications)
