SET(CONVERTDB_SOURCES
        main.cpp
        getopt.cpp
        ../server/database.cpp)

ADD_EXECUTABLE(convertdb ${CONVERTDB_SOURCES})
TARGET_LINK_LIBRARIES(convertdb ${QT_LIBRARIES})

# Installation

INSTALL(TARGETS convertdb RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib COMPONENT applications)

# Make convertdb self-contained wrt. Qt
INSTALL_QT4_EXECUTABLE("bin/convertdb${execext}" "qsqlite" "" "${QT_BINARY_DIR}" "plugins" TRUE applications)
