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
	src/foster_audio.c
)

target_include_directories(${TARGET_NAME} SYSTEM
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/third_party>
)

target_include_directories(${TARGET_NAME}
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# ignore some particularly annoying warnings from our third party libraries
if(MSVC)
	# possible loss of data
	add_definitions("/wd4244 /wd4267")
	# truncation
	add_definitions("/wd4305")
	# signed/unsigned mismatch
	add_definitions("/wd4018")
endif()

file(GLOB_RECURSE TP_DIR "src/third_party/*.c" "src/third_party/*.h")

message(${TP_DIR})

set_source_files_properties(
	${TP_DIR}
	PROPERTIES
    COMPILE_FLAGS
    "-Wno-everything"
)
#add_definitions(-w)

# Get platform target
if (APPLE)
	set(FosterTarget osx)
elseif (WIN32)
	set(FosterTarget x64)
elseif (UNIX)
	set(FosterTarget lib64)
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
endif()

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PRIVATE _UNICODE UNICODE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(${TARGET_NAME} PRIVATE NOMINMAX)
endif ()

# tracks which libraries we need to link, depends on Options above
set(LIBS "")

# Tell SDL it's going to be a shared lib
set_property(TARGET ${FOSTER_SDL2_LIBS} PROPERTY POSITION_INDEPENDENT_CODE ON)

# Link SDL
target_link_libraries(${TARGET_NAME} PRIVATE ${LIBS})
