SET(SERVER_SOURCES
        main.cpp
        database.cpp
        server.cpp
        ../convertdb/getopt.cpp)

SET(SERVER_MOCABLES
        server.h)
SET(SERVER_TS
        ${CMAKE_CURRENT_BINARY_DIR}/server.ts)

QT4_WRAP_CPP(SERVER_MOC_SOURCES ${SERVER_MOCABLES})
QT4_CREATE_TRANSLATION( SERVER_QM ${SERVER_SOURCES} ${SERVER_TS})

ADD_EXECUTABLE(traced ${SERVER_SOURCES} ${SERVER_MOC_SOURCES} ${SERVER_QM})
TARGET_LINK_LIBRARIES(traced ${QT_LIBRARIES})

# Installation
INSTALL(TARGETS traced RUNTIME DESTINATION bin COMPONENT applications
                       ARCHIVE DESTINATION lib COMPONENT applications
                       LIBRARY DESTINATION lib COMPONENT applications)

# Make traced self-contained wrt. Qt
INSTALL_QT4_EXECUTABLE("bin/traced${execext}" "qsqlite" "" "${QT_BINARY_DIR}" "plugins" TRUE applications)
