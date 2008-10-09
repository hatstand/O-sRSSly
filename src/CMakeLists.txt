# Source files
SET(FEEDER-SOURCES
	apiaction.cpp
	atomfeed.cpp
	configuredialog.cpp
	feeditem.cpp
	feedsmodel.cpp
	feedview.cpp
	folderitem.cpp
	readerapi.cpp
	main.cpp
	mainwindow.cpp
	subscriptionlist.cpp
	treeitem.cpp
	xmlutils.cpp
	database.cpp
	settings.cpp
	entryview.cpp
	browser.cpp
	webclipview.cpp
	allitems.cpp
	rootitem.cpp
	keychain.cpp
	longcatbar.cpp
	ksqueezedtextlabel.cpp
	jsonutils.cpp
)

# Header files that have Q_OBJECT in
SET(FEEDER-MOC-HEADERS
	apiaction.h
	configuredialog.h
	feeditem.h
	feedsmodel.h
	feedview.h
	folderitem.h
	mainwindow.h
	readerapi.h
	subscriptionlist.h
	treeitem.h
	settings.h
	entryview.h
	browser.h
	webclipview.h
	allitems.h
	rootitem.h
	ksqueezedtextlabel.h
	longcatbar.h
)

# UI files
SET(FEEDER-UI
	${CMAKE_CURRENT_BINARY_DIR}/configuredialog.ui
	${CMAKE_CURRENT_BINARY_DIR}/mainwindow.ui
	browser.ui
)

# Resource files
SET(FEEDER-RESOURCES
	${CMAKE_CURRENT_BINARY_DIR}/../data/data.qrc
)

CONFIGURE_FILE(configuredialog.ui ${CMAKE_CURRENT_BINARY_DIR}/configuredialog.ui)
CONFIGURE_FILE(mainwindow.ui ${CMAKE_CURRENT_BINARY_DIR}/mainwindow.ui)

SET (EXECUTABLE_OUTPUT_PATH ..)

QT4_WRAP_CPP(FEEDER-SOURCES-MOC ${FEEDER-MOC-HEADERS})
QT4_WRAP_UI(FEEDER-SOURCES-UI ${FEEDER-UI})
QT4_ADD_RESOURCES(FEEDER-SOURCES-RESOURCE ${FEEDER-RESOURCES})
PROTO_GEN(FEEDER-SOURCES-MESSAGES ${FEEDER-MESSAGES})

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QT_QTWEBKIT_INCLUDE_DIR}
	${GNOME_KEYRING_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/../protobuf/src
)

ADD_EXECUTABLE(${BINARY_NAME} ${FEEDER-SOURCES} ${FEEDER-SOURCES-MOC} ${FEEDER-SOURCES-UI} ${FEEDER-SOURCES-RESOURCE} ${FEEDER-SOURCES-MESSAGES})

TARGET_LINK_LIBRARIES(${BINARY_NAME}
	${QT_LIBRARIES}
	${QT_QTWEBKIT_LIBRARY}
	${SECURITY}
	${GNOME_KEYRING_LIBRARIES}
	spawn
)

LINK_DIRECTORIES(${GNOME_KEYRING_LIBRARY_DIRS})

INSTALL(TARGETS ${BINARY_NAME}
	RUNTIME DESTINATION bin)
