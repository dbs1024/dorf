# Copyright (c) Darrin Stewart. All rights reserved.
set(HEADERS
	Include/Core.ArgParser/ArgParser.h
	Include/Core.Base/Assert.h
	Include/Core.Base/Types.h
	Include/Core.Color/Color.h
	Include/Core.Logger/Logger.h
	Include/Core.Math/MathUtil.h
	Include/Core.Math/Vector3f.h
	Include/Core.Math/Vector4f.h
	Include/Core.Memory/FixedItemPool.h
	Include/Core.Memory/RangePool.h
	Include/Core.Memory/SlabAllocator.h
	Include/Core.UnitTest/UnitTest.h
	Include/Core.Vfs/Vfs.h
)

set(SOURCES
	Source/Core.ArgParser/ArgParser.cpp
	Source/Core.Logger/Logger.cpp
	Source/Core.Memory/FixedItemPool.cpp
	Source/Core.Memory/RangePool.cpp
	Source/Core.Memory/SlabAllocator.cpp
	Source/Core.UnitTest/UnitTest.cpp
	Source/Core.Vfs/DiskFileSystem.cpp
	Source/Core.Vfs/Vfs.cpp
)

add_library(Ace.Core STATIC ${HEADERS} ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES})

target_include_directories(Ace.Core
	PUBLIC  Include
	PRIVATE Source
)
