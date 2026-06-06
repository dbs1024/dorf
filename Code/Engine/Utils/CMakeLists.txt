# Copyright (c) Darrin Stewart. All rights reserved.
set(HEADERS
	Include/Engine.Utils.XmlDoc/XmlDoc.h
)

set(SOURCES
	Source/Engine.Utils.XmlDoc/XmlDocInternal.h
	Source/Engine.Utils.XmlDoc/XmlDoc.cpp
	Source/Engine.Utils.XmlDoc/XmlLoader.cpp
)

add_library(Ace.Engine.Utils STATIC ${HEADERS} ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES})

target_include_directories(Ace.Engine.Utils
	PUBLIC  Include
	PRIVATE Source
)
