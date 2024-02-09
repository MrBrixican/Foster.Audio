cmake_minimum_required(VERSION 3.14)
project(FosterAudioPlatform C)

# Set flag for building a universal binary on macOS
if(APPLE)
	set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

# Default to Release build for library
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# Define target name
set (TARGET_NAME FosterAudioPlatform)

add_library(${TARGET_NAME} SHARED
	include/foster_platform.h
	src/foster_platform.c
)

target_include_directories(${TARGET_NAME} SYSTEM
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/third_party>
)

target_include_directories(${TARGET_NAME}
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# miniaudio dependencies
# windows: none
# mac: none
# linux: -lpthread, -lm, -ldl
# bsd: -lpthread, -lm

if (UNIX AND NOT APPLE)
   set(THREADS_PREFER_PTHREAD_FLAG ON)
   find_package(Threads REQUIRED)
   target_link_libraries(${TARGET_NAME} PRIVATE Threads::Threads ${LIB_M})
   find_library(LIB_M m)
   if (LIB_M)
       target_link_libraries(${TARGET_NAME} PRIVATE ${LIB_M})
   endif()
   if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
       target_link_libraries(${TARGET_NAME} PRIVATE ${CMAKE_DL_LIBS})
   endif ()
endif ()

# Get platform target
if (APPLE)
	set(FosterTarget osx)
elseif (WIN32)
	set(FosterTarget x64)
elseif (UNIX)
	if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
		set(FosterTarget libarm64)
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv7l")
		set(FosterTarget libarm)
	elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv6l")
		set(FosterTarget libarm)
	else()
		set(FosterTarget lib64)
	endif()
else()
	message(FATAL_ERROR "Unknown Target!")
endif()

# Output libs to platform-specific library
set_target_properties(${TARGET_NAME}
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libs/${FosterTarget}"
	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/libs/${FosterTarget}"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libs/${FosterTarget}"
	LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/libs/${FosterTarget}"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libs/${FosterTarget}"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/libs/${FosterTarget}"
)

if (MSVC)
    # Set warning level 3
    target_compile_options(${TARGET_NAME} PRIVATE /W3)

    # Warnings as Errors Enabled
    target_compile_options(${TARGET_NAME} PRIVATE /WX)

	# ignore some particularly annoying warnings from our third party libraries:
	# possible loss of data
	add_definitions("/wd4244 /wd4267")
	# truncation
	add_definitions("/wd4305")
	# signed/unsigned mismatch
	add_definitions("/wd4018")
endif()

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE _UNICODE UNICODE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX)
endif ()
