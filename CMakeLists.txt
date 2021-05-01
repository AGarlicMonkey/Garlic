cmake_minimum_required(VERSION 3.18.0)

enable_testing()

project(Garlic LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.7.2")

#Garlic settings
option(GAR_BUILD_BULB "Build Bulb, Garlic's editor" ON)
option(GAR_BUILD_TESTS "Build Garlic's tests" OFF)
option(GAR_MANUALLY_INSTALL_PACKAGES "Stop CMake automatically installing conan packages when configuring" OFF)

#Required for linking shared object (*.so) files on Linux
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#Set up conan
if(NOT GAR_MANUALLY_INSTALL_PACKAGES)
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   		message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   		file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
        	          "${CMAKE_BINARY_DIR}/conan.cmake")
	endif()
	include(${CMAKE_BINARY_DIR}/conan.cmake)

	if(${GENERATOR_IS_MULTI_CONFIG})
		conan_cmake_run(CONANFILE conanfile.txt BASIC_SETUP CMAKE_TARGETS BUILD missing CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}")
	else()
		conan_cmake_run(CONANFILE conanfile.txt BASIC_SETUP CMAKE_TARGETS BUILD missing BUILD_TYPE "${CMAKE_BUILD_TYPE}")
	endif()	
else()
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup(TARGETS)
endif()

#Engine
add_subdirectory(clove)

#Editor
if(${GAR_BUILD_BULB} AND ${MSVC})
	add_subdirectory(bulb)
endif()