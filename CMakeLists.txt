# Copyright (c) Darrin Stewart. All rights reserved.
cmake_minimum_required(VERSION 3.20)

project(Ace)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

add_subdirectory(Code/Core)
add_subdirectory(Code/Core/Tests)
add_subdirectory(Code/Engine/Base)
add_subdirectory(Code/Engine/Client)
add_subdirectory(Code/Engine/Tests)
add_subdirectory(Code/Engine/Utils)
add_subdirectory(Code/Examples/BasicDraw)
add_subdirectory(Code/Tools/AssetForge)
add_subdirectory(Code/Tools/UnitTestRunner)

