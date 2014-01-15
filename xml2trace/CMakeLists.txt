SET(TRACE2XML_SOURCES
        main.cpp
        ../server/xmlcontenthandler.cpp
        ../server/databasefeeder.cpp
        ../convertdb/getopt.cpp
        ../server/database.cpp)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)

ADD_EXECUTABLE(xml2trace MACOSX_BUNDLE ${TRACE2XML_SOURCES})
TARGET_LINK_LIBRARIES(xml2trace ${QT_LIBRARIES})

INSTALL(TARGETS xml2trace RUNTIME DESTINATION bin COMPONENT applications
                          LIBRARY DESTINATION lib COMPONENT applications
                          BUNDLE  DESTINATION bin COMPONENT applications
                          ARCHIVE DESTINATION lib COMPONENT applications)
# Make xml2trace self-contained wrt. Qt
INSTALL_QT4_EXECUTABLE("bin/xml2trace${execext}" "qsqlite" "" "${QT_BINARY_DIR}" "plugins" TRUE applications)
