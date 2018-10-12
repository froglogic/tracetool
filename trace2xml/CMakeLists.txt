SET(TRACE2XML_SOURCES
        main.cpp
        ../server/database.cpp)

IF(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)

ADD_EXECUTABLE(trace2xml MACOSX_BUNDLE ${TRACE2XML_SOURCES})
TARGET_LINK_LIBRARIES(trace2xml Qt5::Sql)

INSTALL(TARGETS trace2xml RUNTIME DESTINATION bin COMPONENT applications
                          LIBRARY DESTINATION lib COMPONENT applications
                          BUNDLE  DESTINATION bin COMPONENT applications
                          ARCHIVE DESTINATION lib COMPONENT applications)

IF(APPLE AND BUNDLE_QT)
    GET_TARGET_PROPERTY(_qmake_path Qt5::qmake IMPORTED_LOCATION)
    GET_FILENAME_COMPONENT(_qt_bindir ${_qmake_path} DIRECTORY)
    INSTALL(
        CODE "EXECUTE_PROCESS(COMMAND \"${_qt_bindir}/macdeployqt\" \"\${CMAKE_INSTALL_PREFIX}/bin/trace2xml.app\")"
        COMPONENT applications)
ENDIF()