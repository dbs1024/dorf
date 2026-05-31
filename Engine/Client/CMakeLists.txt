# Copyright (c) Darrin Stewart. All rights reserved.
set(HEADERS
	Include/Engine.DemoApp/DemoApp.h
)

set(SOURCES
	Source/Engine.DemoApp/DemoApp.cpp
)

add_library(Ace.Engine.Client STATIC ${HEADERS} ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES})

target_include_directories(Ace.Engine.Client
	PUBLIC  Include
	PRIVATE Source
)
