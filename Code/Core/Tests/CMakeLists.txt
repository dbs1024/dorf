# Copyright (c) Darrin Stewart. All rights reserved.
add_library(Ace.Core.Tests SHARED
	UnitTestCore.cpp
	UnitTestArgParser.cpp
	UnitTestColor.cpp
	UnitTestMath.cpp
	UnitTestMisc.cpp
)

target_link_libraries(Ace.Core.Tests PRIVATE Ace.Core)
