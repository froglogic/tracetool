SET(TRACE2XML_SOURCES
        main.cpp
        ../convertdb/getopt.cpp
        ../server/database.cpp)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)

ADD_EXECUTABLE(trace2xml ${TRACE2XML_SOURCES})
TARGET_LINK_LIBRARIES(trace2xml ${QT_LIBRARIES})

INSTALL(TARGETS trace2xml RUNTIME DESTINATION bin
                          LIBRARY DESTINATION lib
                          ARCHIVE DESTINATION lib COMPONENT devel)
# Make trace2xml self-contained wrt. Qt
INSTALL_QT4_EXECUTABLE("bin/trace2xml${execext}" "qsqlite" "" "${QT_BINARY_DIR}" "plugins" TRUE devel)
