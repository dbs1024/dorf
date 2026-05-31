# Copyright (c) Darrin Stewart. All rights reserved.
set(HEADERS
	Include/Engine.DemoApp/DemoApp.h
	Include/Engine.Render/Rhi/RhiDevice.h
)

set(SOURCES
	Source/Engine.DemoApp/DemoApp.cpp
	Source/Engine.Render/D3D12/D3D12Device.cpp
)

add_library(Ace.Engine.Client STATIC ${HEADERS} ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES})

target_include_directories(Ace.Engine.Client
	PUBLIC  Include
	PRIVATE Source
)

target_link_libraries(Ace.Engine.Client
	PRIVATE d3d12.lib dxgi.lib
)
