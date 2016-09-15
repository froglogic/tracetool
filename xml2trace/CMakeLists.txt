SET(TRACE2XML_SOURCES
        main.cpp
        ../server/xmlcontenthandler.cpp
        ../server/databasefeeder.cpp
        ../server/database.cpp)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)

ADD_EXECUTABLE(xml2trace MACOSX_BUNDLE ${TRACE2XML_SOURCES})
TARGET_LINK_LIBRARIES(xml2trace Qt5::Sql)

INSTALL(TARGETS xml2trace RUNTIME DESTINATION bin COMPONENT applications
                          LIBRARY DESTINATION lib COMPONENT applications
                          BUNDLE  DESTINATION bin COMPONENT applications
                          ARCHIVE DESTINATION lib COMPONENT applications)
