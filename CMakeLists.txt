# Copyright (c) Darrin Stewart. All rights reserved.
cmake_minimum_required(VERSION 3.20)

project(Ace)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(Core)
add_subdirectory(Core/Tests)
add_subdirectory(Tools/UnitTestRunner)
