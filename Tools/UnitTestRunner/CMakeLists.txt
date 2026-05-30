# Copyright (c) Darrin Stewart. All rights reserved.
add_executable(Ace.Tools.UnitTestRunner
	UnitTestRunner.cpp
)

target_link_libraries(Ace.Tools.UnitTestRunner
	PRIVATE Ace.Core
	PRIVATE dbghelp
)
