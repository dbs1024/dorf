# Copyright (c) Darrin Stewart. All rights reserved.
set(HEADERS
	Include/Engine.Render.Base/RhiDecls.h
)

add_library(Ace.Engine.Base INTERFACE)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS})

target_include_directories(Ace.Engine.Base
	INTERFACE Include
)
