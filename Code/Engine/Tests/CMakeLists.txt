# Copyright (c) Darrin Stewart. All rights reserved.
add_library(Ace.Engine.Tests SHARED
	UnitTestEngineUtils.cpp
	UnitTestXmlDoc.cpp
)

target_link_libraries(Ace.Engine.Tests
	PRIVATE Ace.Core
	PRIVATE Ace.Engine.Utils
)
