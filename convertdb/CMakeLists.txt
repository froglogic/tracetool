SET(CONVERTDB_SOURCES
        main.cpp
        getopt.cpp
        ../server/database.cpp)

ADD_EXECUTABLE(convertdb ${CONVERTDB_SOURCES})
TARGET_LINK_LIBRARIES(convertdb ${QT_LIBRARIES})

# Installation

INSTALL(TARGETS convertdb RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib COMPONENT devel)

