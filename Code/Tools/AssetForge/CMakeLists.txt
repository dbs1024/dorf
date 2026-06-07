# Copyright (c) Darrin Stewart. All rights reserved.
add_executable(Ace.Tools.AssetForge
	AssetForge.cpp
)

target_link_libraries(Ace.Tools.AssetForge
	PRIVATE Ace.Core
	PRIVATE Ace.Engine.AssetPipeline
	PRIVATE Ace.Engine.Utils
)
