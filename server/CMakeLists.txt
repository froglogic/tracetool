SET(SERVER_SOURCES
        main.cpp
        database.cpp
        server.cpp
        databasefeeder.cpp
        xmlcontenthandler.cpp)

SET(SERVER_TS
        ${CMAKE_CURRENT_BINARY_DIR}/server.ts)

QT5_CREATE_TRANSLATION( SERVER_QM ${SERVER_SOURCES} ${SERVER_TS})

ADD_EXECUTABLE(traced MACOSX_BUNDLE ${SERVER_SOURCES} ${SERVER_QM})
TARGET_LINK_LIBRARIES(traced Qt5::Core Qt5::Network Qt5::Sql)

# Installation
INSTALL(TARGETS traced RUNTIME DESTINATION bin COMPONENT applications
                       ARCHIVE DESTINATION lib COMPONENT applications
                       BUNDLE  DESTINATION bin COMPONENT applications
                       LIBRARY DESTINATION lib COMPONENT applications)

IF(APPLE AND BUNDLE_QT)
    GET_TARGET_PROPERTY(_qmake_path Qt5::qmake IMPORTED_LOCATION)
    GET_FILENAME_COMPONENT(_qt_bindir ${_qmake_path} DIRECTORY)
    INSTALL(
        CODE "EXECUTE_PROCESS(COMMAND \"${_qt_bindir}/macdeployqt\" \"\${CMAKE_INSTALL_PREFIX}/bin/traced.app\")"
        COMPONENT applications)
ENDIF()