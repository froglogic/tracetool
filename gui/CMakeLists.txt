SET(GUI_SOURCES
  main.cpp
  settings.cpp
  mainwindow.cpp
  entryfilter.cpp
  filterform.cpp
  entryitemmodel.cpp
  watchtree.cpp
  ../convertdb/getopt.cpp
  ../server/database.cpp
  ../server/server.cpp)

SET(GUI_MOCABLES
  mainwindow.h
  entryfilter.h
  filterform.h
  entryitemmodel.h
  watchtree.h
  ../server/server.h)

SET(GUI_UIS
  mainwindow.ui
  filterform.ui)

SET(GUI_RCCS 
  gui.qrc)

FIND_PACKAGE(Qt4 4.5.0 COMPONENTS QtGui QtCore QtSql QtNetwork QtXml QtSql REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(GUI_MOC_SOURCES ${GUI_MOCABLES})
QT4_WRAP_UI(GUI_UI_HDRS ${GUI_UIS} )
QT4_ADD_RESOURCES(GUI_RCC_SRCS ${GUI_RCCS})

# Locate generated ui_*.f files in build directory
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/gui)

ADD_EXECUTABLE(tracegui ${GUI_SOURCES} ${GUI_MOC_SOURCES} ${GUI_UI_HDRS} ${GUI_RCC_SRCS})
TARGET_LINK_LIBRARIES(tracegui ${QT_LIBRARIES})

# Installation
INSTALL(TARGETS tracegui RUNTIME DESTINATION bin
                         LIBRARY DESTINATION lib
                         ARCHIVE DESTINATION lib COMPONENT devel)
