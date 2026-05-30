# Copyright (c) Darrin Stewart. All rights reserved.
set(HEADERS
	Include/Core.ArgParser/ArgParser.h
	Include/Core.Base/Assert.h
	Include/Core.UnitTest/UnitTest.h
	Include/Core.Util/FixedItemPool.h
)

set(SOURCES
	Source/Core.ArgParser/ArgParser.cpp
	Source/Core.UnitTest/UnitTest.cpp
	Source/Core.Util/FixedItemPool.cpp
)

add_library(Ace.Core STATIC ${HEADERS} ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES})

target_include_directories(Ace.Core
	PUBLIC  Include
	PRIVATE Source
)
