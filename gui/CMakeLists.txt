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
# Make tracegui self-contained wrt. Qt
# INSTALL_QT4_EXECUTABLE("bin/tracegui${execext}" "qsqlite" "" "${QT_BINARY_DIR}" "plugins" TRUE applications)
