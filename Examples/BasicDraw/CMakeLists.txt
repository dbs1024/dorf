# Copyright (c) Darrin Stewart. All rights reserved.
add_executable(Ace.Examples.BasicDraw
	BasicDraw.cpp
)

target_link_libraries(Ace.Examples.BasicDraw
	PRIVATE Ace.Engine.Client
)
