# Copyright (c) Darrin Stewart. All rights reserved.
set(HEADERS
	Include/Engine.AssetPipeline/AssetPipeline.h
)

set(SOURCES
	Source/Engine.AssetPipeline/AssetPipeline.cpp
)

add_library(Ace.Engine.AssetPipeline STATIC ${HEADERS} ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES})

target_include_directories(Ace.Engine.AssetPipeline
	PUBLIC  Include
	PRIVATE Source
)

target_link_libraries(Ace.Engine.AssetPipeline
	PRIVATE Ace.Core
)
