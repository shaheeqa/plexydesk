FIND_PACKAGE(TelepathyQt4)

INCLUDE_DIRECTORIES(
    ${TELEPATHY_QT4_INCLUDE_DIR}
    )

set(sourceFiles
    presence.cpp
    presenceinterface.cpp
    )

set(headerFiles
    presence.h
    presenceinterface.h
    )

set(QTMOC_SRCS
    presence.h
    presenceinterface.h
    )

QT4_WRAP_CPP(QT_MOC_SRCS ${QTMOC_SRCS})

set(sourceFiles
    ${sourceFiles}
    ${headerFiles}
    )

set(libs
    ${QT_QTGUI_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${TELEPATHY_QT4_LIBRARIES}
    )

ADD_LIBRARY(presenceengine SHARED ${sourceFiles} ${QT_MOC_SRCS})

TARGET_LINK_LIBRARIES(presenceengine
    qtviz
    ${libs}
    )

INSTALL(TARGETS presenceengine DESTINATION ${CMAKE_INSTALL_LIBDIR}/plexyext)
INSTALL(FILES presence.desktop DESTINATION share/plexy/ext/groups)
