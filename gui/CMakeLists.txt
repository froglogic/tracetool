SET(GUI_SOURCES
  main.cpp
  settings.cpp
  mainwindow.cpp
  fixedheaderview.cpp
  entryfilter.cpp
  columnsinfo.cpp
  filterform.cpp
  settingsform.cpp
  storageview.cpp
  configuration.cpp
  configeditor.cpp
  entryitemmodel.cpp
  watchtree.cpp
  applicationtable.cpp
  searchwidget.cpp
  ../server/database.cpp)

SET(GUI_MOCABLES
  mainwindow.h
  fixedheaderview.h
  entryfilter.h
  columnsinfo.h
  filterform.h
  settingsform.h
  storageview.h
  configuration.h
  configeditor.h
  entryitemmodel.h
  watchtree.h
  applicationtable.h
  searchwidget.h)

SET(GUI_UIS
  mainwindow.ui
  settingsform.ui
  storageview.ui
  configeditor.ui
  filterform.ui)

SET(GUI_RCCS 
  gui.qrc)

SET(GUI_TS
    ${CMAKE_CURRENT_BINARY_DIR}/gui.ts)

IF (WIN32)
    LIST(APPEND GUI_SOURCES jobobject.cpp)
ENDIF()

IF (FALSE)
    LIST(APPEND GUI_SOURCES modeltest.cpp)
    LIST(APPEND GUI_MOCABLES modeltest.h)
    ADD_DEFINITIONS(-DHAVE_MODELTEST)
ENDIF()

# Make sure QtGui is available by finding Qt again
FIND_PACKAGE(Qt5 COMPONENTS Core Network Xml Sql Gui Widgets REQUIRED)

INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})

QT5_WRAP_CPP(GUI_MOC_SOURCES ${GUI_MOCABLES})
QT5_WRAP_UI(GUI_UI_HDRS ${GUI_UIS} )
QT5_ADD_RESOURCES(GUI_RCC_SRCS ${GUI_RCCS})
QT5_CREATE_TRANSLATION( GUI_QM ${GUI_SOURCES} ${GUI_RCCS} ${GUI_UIS} ${GUI_TS} )

# Locate generated ui_*.f files in build directory
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${QT_QTGUI_INCLUDE_DIR})

ADD_EXECUTABLE(tracegui WIN32 MACOSX_BUNDLE ${GUI_SOURCES} ${GUI_MOC_SOURCES} ${GUI_UI_HDRS} ${GUI_RCC_SRCS} ${GUI_QM})

SET_TARGET_PROPERTIES(tracegui PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
)

TARGET_LINK_LIBRARIES(tracegui Qt5::Gui Qt5::Widgets Qt5::Sql Qt5::Network)

# Installation
INSTALL(TARGETS tracegui RUNTIME DESTINATION bin COMPONENT applications
                         LIBRARY DESTINATION lib COMPONENT applications
                         BUNDLE  DESTINATION bin COMPONENT applications
                         ARCHIVE DESTINATION lib COMPONENT applications)
# Make tracegui (and other executables in same directory) self-contained wrt. Qt
IF(WIN32)
    GET_TARGET_PROPERTY(_qmake_path Qt5::qmake IMPORTED_LOCATION)
    GET_FILENAME_COMPONENT(_qt_bindir ${_qmake_path} DIRECTORY)
    INSTALL(
        CODE "EXECUTE_PROCESS(COMMAND \"${_qt_bindir}/windeployqt.exe\" \"\${CMAKE_INSTALL_PREFIX}/bin/tracegui.exe\")"
        COMPONENT applications)
ELSEIF(NOT WIN32 AND NOT APPLE AND BUNDLE_QT)
    FUNCTION(install_qt5_plugin pluginTargetName)
        STRING(TOUPPER ${CMAKE_BUILD_TYPE} _buildType_UC)
        GET_TARGET_PROPERTY(_qtplugin_path ${pluginTargetName} IMPORTED_LOCATION_${_buildType_UC})
        GET_FILENAME_COMPONENT(_qtplugin_name "${_qtplugin_path}" NAME)
        GET_FILENAME_COMPONENT(_qtplugin_dir "${_qtplugin_path}" DIRECTORY)
        GET_FILENAME_COMPONENT(_qtplugin_type "${_qtplugin_dir}" NAME)
        SET(${installPathVar} "\${CMAKE_INSTALL_PREFIX}/plugins/${_qtplugin_type}/${_qtplugin_name}")
        INSTALL(FILES "${_qtplugin_path}" DESTINATION "plugins/${_qtplugin_type}" COMPONENT applications)
    ENDFUNCTION()
    FUNCTION(install_library_from_qt_libdir libname)
        STRING(TOUPPER ${CMAKE_BUILD_TYPE} _buildType_UC)
        GET_TARGET_PROPERTY(_qtlib_dir Qt5::Core IMPORTED_LOCATION_${_buildType_UC})
        GET_FILENAME_COMPONENT(_qtlib_dir ${_qtlib_dir} PATH)
        FILE(GLOB _libfiles "${_qtlib_dir}/${libname}*")
        message(STATUS "${_libfiles}")
        INSTALL(FILES ${_libfiles} DESTINATION lib COMPONENT applications)
    ENDFUNCTION()
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libQt5Core.so.5")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libQt5DBus.so.5")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libQt5Gui.so.5")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libQt5Network.so.5")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libQt5Sql.so.5")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libQt5Widgets.so.5")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libQt5XcbQpa.so.5")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libicudata.so.56")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libicuuc.so.56")
    INSTALL_LIBRARY_FROM_QT_LIBDIR("libicui18n.so.56")
    INSTALL_QT5_PLUGIN(Qt5::QXcbIntegrationPlugin)
    INSTALL_QT5_PLUGIN(Qt5::QSQLiteDriverPlugin)
    INSTALL(
        CODE "FILE(WRITE \"\${CMAKE_INSTALL_PREFIX}/bin/qt.conf\" \"[Paths]\nPrefix=..\")"
        COMPONENT applications )
ENDIF()
