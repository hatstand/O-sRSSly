# Source files
SET(SPAWN-SOURCES
	manager.cpp
	view.cpp
	child.cpp
	spawn.cpp
	iodevicestream.cpp
	qthelpers.cpp
	page.cpp
)

SET(SPAWN-SOURCES-UNIX
	mapped_memory_unix.cpp
)

SET(SPAWN-SOURCES-WIN32
	mapped_memory_win32.cpp
)

# Header files that have Q_OBJECT in
SET(SPAWN-MOC-HEADERS
	manager.h
	view.h
	child.h
	spawn.h
	page.h
)

# Resource files
SET(SPAWN-RESOURCES
	data/spawn.qrc
)

# protobuf messages
SET(SPAWN-MESSAGES
	../messages/spawnevent.proto
	../messages/inputevents.proto
	../messages/controlevents.proto
	../messages/spawnreply.proto
)

IF(UNIX)
	SET(SPAWN-SOURCES ${SPAWN-SOURCES} ${SPAWN-SOURCES-UNIX})
ELSE(UNIX)
	SET(SPAWN-SOURCES ${SPAWN-SOURCES} ${SPAWN-SOURCES-WIN32})
ENDIF(UNIX)

SET (EXECUTABLE_OUTPUT_PATH ..)

QT4_WRAP_CPP(SPAWN-SOURCES-MOC ${SPAWN-MOC-HEADERS})
QT4_ADD_RESOURCES(SPAWN-SOURCES-RESOURCE ${SPAWN-RESOURCES})
PROTO_GEN(SPAWN-SOURCES-MESSAGES ${SPAWN-MESSAGES})

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QT_QTWEBKIT_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/../protobuf/src
)

ADD_LIBRARY(spawn STATIC
	${SPAWN-SOURCES}
	${SPAWN-SOURCES-MOC}
	${SPAWN-SOURCES-MESSAGES}
	${SPAWN-SOURCES-RESOURCE}
)

TARGET_LINK_LIBRARIES(spawn
	${QT_LIBRARIES}
	${QT_QTWEBKIT_LIBRARY}
	protobuf
)
