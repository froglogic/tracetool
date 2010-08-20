SET(GUI_SOURCES
  main.cpp
  entryitemmodel.cpp
  ../server/server.cpp)

SET(GUI_MOCABLES
  entryitemmodel.h
  ../server/server.h)

FIND_PACKAGE(Qt4 4.5.0 COMPONENTS QtGui QtCore QtSql QtNetwork QtXml QtSql REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(GUI_MOC_SOURCES ${GUI_MOCABLES})

ADD_EXECUTABLE(tracegui ${GUI_SOURCES} ${GUI_MOC_SOURCES})
TARGET_LINK_LIBRARIES(tracegui ${QT_LIBRARIES})
