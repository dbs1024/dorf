# Copyright (c) Darrin Stewart. All rights reserved.
set(HEADERS
	Include/Engine.DemoApp/DemoApp.h
	Include/Engine.Input/InputManager.h
	Include/Engine.Render/DebugDraw.h
	Include/Engine.Render/Rhi/RhiDevice.h
)

set(SOURCES
	Source/Engine.DemoApp/DemoApp.cpp
	Source/Engine.Input/InputManager.cpp
	Source/Engine.Render/DebugDraw.cpp
	Source/Engine.Render/D3D12/D3D12Device.h
	Source/Engine.Render/D3D12/D3D12Device.cpp
	Source/Engine.Render/D3D12/D3D12CommandList.cpp
	Source/Engine.Render/D3D12/D3D12CommandQueue.cpp
	Source/Engine.Render/D3D12/D3D12DescriptorHeap.cpp
	Source/Engine.Render/D3D12/D3D12RingBuffer.cpp
	Source/Engine.Render/D3D12/D3D12Sampler.cpp
)

add_library(Ace.Engine.Client STATIC ${HEADERS} ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES})

target_include_directories(Ace.Engine.Client
	PUBLIC  Include
	PRIVATE Source
)

target_link_libraries(Ace.Engine.Client
	PUBLIC  Ace.Core
	PRIVATE d3d12.lib dxgi.lib
)
