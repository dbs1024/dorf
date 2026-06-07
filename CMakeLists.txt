# Copyright (c) Darrin Stewart. All rights reserved.
cmake_minimum_required(VERSION 3.20)

project(Ace)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

#----------------------------------------------------------------------------------------------------------------------
# Utility functions
#----------------------------------------------------------------------------------------------------------------------

# Helper to download and unzip a package from a URL
# Uses a zero-length file to identify the version of the package
# use example:
# CheckAndDownloadPackage("DXC" "v1.7.2308" ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2308/dxc_2023_08_14.zip "zip")
function(CheckAndDownloadPackage NAME VERSION LOCAL_PATH URL ARCHIVE_TYPE)
	# Do we already have the correct version?
	if(NOT EXISTS ${LOCAL_PATH}/${VERSION}.ver)
		# Was there a previous version that we need to delete?
		if(EXISTS ${LOCAL_PATH})
			message(STATUS "Deleting old " ${NAME})
			file(REMOVE_RECURSE ${LOCAL_PATH})
		endif()
		message(STATUS "Obtaining " ${NAME} " " ${VERSION} " from " ${URL})
		file(DOWNLOAD ${URL} ${LOCAL_PATH}.${ARCHIVE_TYPE})
		message(STATUS "Extracting " ${NAME} " to " ${LOCAL_PATH})
		file(ARCHIVE_EXTRACT INPUT ${LOCAL_PATH}.${ARCHIVE_TYPE} DESTINATION ${LOCAL_PATH})
		file(REMOVE ${LOCAL_PATH}.${ARCHIVE_TYPE})
		# Create an empty file so we know which version we have
		file(WRITE ${LOCAL_PATH}/${VERSION}.ver)
	endif()
endfunction()

#----------------------------------------------------------------------------------------------------------------------
# External packages
#----------------------------------------------------------------------------------------------------------------------

set(DXC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/dxc)
CheckAndDownloadPackage("DXC" "v1.9.2602.24" ${DXC_PATH} https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2602.24/dxc_2026_05_27.zip "zip")
set(DXC_BIN_PATH ${DXC_PATH}/bin/x64)
set(DXC_LIB_PATH ${DXC_PATH}/lib/x64)
set(DXC_INC_PATH ${DXC_PATH}/inc)

# Copy the DXC runtime DLLs alongside the built executables for each configuration
foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
	file(COPY ${DXC_BIN_PATH}/dxcompiler.dll ${DXC_BIN_PATH}/dxil.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG_TYPE})
endforeach()

#----------------------------------------------------------------------------------------------------------------------
# Ace Engine projects
#----------------------------------------------------------------------------------------------------------------------

add_subdirectory(Code/Core)
add_subdirectory(Code/Core/Tests)
add_subdirectory(Code/Engine/Base)
add_subdirectory(Code/Engine/Client)
add_subdirectory(Code/Engine/Pipeline)
add_subdirectory(Code/Engine/Tests)
add_subdirectory(Code/Engine/Utils)
add_subdirectory(Code/Examples/BasicDraw)
add_subdirectory(Code/Tools/AssetForge)
add_subdirectory(Code/Tools/UnitTestRunner)

