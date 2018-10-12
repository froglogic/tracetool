SET(CONVERTDB_SOURCES
        main.cpp
        ../server/database.cpp)

ADD_EXECUTABLE(convertdb MACOSX_BUNDLE ${CONVERTDB_SOURCES})
TARGET_LINK_LIBRARIES(convertdb Qt5::Sql)

# Installation

INSTALL(TARGETS convertdb RUNTIME DESTINATION bin COMPONENT applications
                          LIBRARY DESTINATION lib COMPONENT applications
                          BUNDLE  DESTINATION bin COMPONENT applications
                          ARCHIVE DESTINATION lib COMPONENT applications)

IF(APPLE AND BUNDLE_QT)
    GET_TARGET_PROPERTY(_qmake_path Qt5::qmake IMPORTED_LOCATION)
    GET_FILENAME_COMPONENT(_qt_bindir ${_qmake_path} DIRECTORY)
    INSTALL(
        CODE "EXECUTE_PROCESS(COMMAND \"${_qt_bindir}/macdeployqt\" \"\${CMAKE_INSTALL_PREFIX}/bin/convertdb.app\")"
        COMPONENT applications)
ENDIF()