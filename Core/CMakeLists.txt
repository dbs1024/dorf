add_library(Dorf.Core STATIC
	Source/Core.UnitTest/UnitTest.cpp
)

target_include_directories(Dorf.Core
	PUBLIC  Include
	PRIVATE Source
)
