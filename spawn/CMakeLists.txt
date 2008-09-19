# Source files
SET(SPAWN-SOURCES
	manager.cpp
	view.cpp
	child.cpp
)

# Header files that have Q_OBJECT in
SET(SPAWN-MOC-HEADERS
	manager.h
	view.h
	child.h
)

# protobuf messages
SET(SPAWN-MESSAGES
	../messages/spawnevent.proto
	../messages/mouseevent.proto
	../messages/point.proto
)

SET (EXECUTABLE_OUTPUT_PATH ..)

QT4_WRAP_CPP(SPAWN-SOURCES-MOC ${SPAWN-MOC-HEADERS})
PROTO_GEN(SPAWN-SOURCES-MESSAGES ${SPAWN-MESSAGES})

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${QT_QTWEBKIT_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../protobuf/src
)

ADD_LIBRARY(spawn STATIC
	${SPAWN-SOURCES}
	${SPAWN-SOURCES-MOC}
	${SPAWN-SOURCES-MESSAGES}
)

TARGET_LINK_LIBRARIES(spawn
	${QT_LIBRARIES}
	${QT_QTWEBKIT_LIBRARY}
	protobuf
)
