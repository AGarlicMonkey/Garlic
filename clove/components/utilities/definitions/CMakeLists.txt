#CMake
option(CLOVE_ENABLE_DEBUG_DEFINITION "Sets the CLOVE_DEBUG macro to 1. Allowing for various debug functionality / information to be enabled" OFF)

#Library
add_library(
	CloveDefinitions INTERFACE
)

#Include
target_include_directories(
	CloveDefinitions
	
	INTERFACE 
		include
)

#Preprocessor
target_compile_definitions(
	CloveDefinitions

	INTERFACE
		#Platform
		CLOVE_PLATFORM_WINDOWS=$<PLATFORM_ID:Windows>
		CLOVE_PLATFORM_LINUX=$<PLATFORM_ID:Linux>
		CLOVE_PLATFORM_MACOS=$<PLATFORM_ID:Darwin>

		#Flags
		CLOVE_DEBUG=$<BOOL:${CLOVE_ENABLE_DEBUG_DEFINITION}>
)