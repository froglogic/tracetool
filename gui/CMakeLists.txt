SET(GUI_SOURCES
  main.cpp
  settings.cpp
  mainwindow.cpp
  entryfilter.cpp
  columnsinfo.cpp
  filterform.cpp
  columnsform.cpp
  storageview.cpp
  configuration.cpp
  configeditor.cpp
  entryitemmodel.cpp
  watchtree.cpp
  searchwidget.cpp
  ../convertdb/getopt.cpp
  ../server/database.cpp
  ../server/server.cpp)

SET(GUI_MOCABLES
  mainwindow.h
  entryfilter.h
  columnsinfo.h
  filterform.h
  columnsform.h
  storageview.h
  configuration.h
  configeditor.h
  entryitemmodel.h
  watchtree.h
  searchwidget.h
  ../server/server.h)

SET(GUI_UIS
  mainwindow.ui
  columnsform.ui
  storageview.ui
  configeditor.ui
  filterform.ui)

SET(GUI_RCCS 
  gui.qrc)

SET(GUI_TS
    ${CMAKE_CURRENT_BINARY_DIR}/gui.ts)

FIND_PACKAGE(Qt4 4.6 COMPONENTS QtGui QtCore QtSql QtNetwork QtXml QtSql REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(GUI_MOC_SOURCES ${GUI_MOCABLES})
QT4_WRAP_UI(GUI_UI_HDRS ${GUI_UIS} )
QT4_ADD_RESOURCES(GUI_RCC_SRCS ${GUI_RCCS})
QT4_CREATE_TRANSLATION( GUI_QM ${GUI_SOURCES} ${GUI_RCCS} ${GUI_UIS} ${GUI_TS} )

# Locate generated ui_*.f files in build directory
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR})

ADD_EXECUTABLE(tracegui ${GUI_SOURCES} ${GUI_MOC_SOURCES} ${GUI_UI_HDRS} ${GUI_RCC_SRCS} ${GUI_QM})
TARGET_LINK_LIBRARIES(tracegui ${QT_LIBRARIES})

# Installation
INSTALL(TARGETS tracegui RUNTIME DESTINATION bin
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib COMPONENT devel)
