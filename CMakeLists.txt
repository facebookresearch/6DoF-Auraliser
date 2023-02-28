cmake_minimum_required(VERSION 3.15)

project(playground VERSION 1.0.0)

# Build everything as "Release" by default
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

# Configure CMake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Platform specific flags and definitions
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
elseif(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall -Wextra -Wno-inconsistent-missing-override")  
elseif(UNIX AND NOT APPLE AND NOT ANDROID)
    add_definitions(
        #-std=c++11
        -DLINUX=1   
    )
endif()

# Disable JUCE extras and examples:
option(JUCE_BUILD_EXTRAS   "Build JUCE Extras"   OFF)
option(JUCE_BUILD_EXAMPLES "Build JUCE Examples" OFF)

# Disable SAF tests and examples:
option(SAF_BUILD_TESTS    "Build SAF unit tests." OFF)
option(SAF_BUILD_EXAMPLES "Build SAF examples."   OFF)

# Enabled SAF sofa reader and HADES modules
option(SAF_ENABLE_SOFA_READER_MODULE "Enable SOFA reader SAF module."  ON)
option(SAF_ENABLE_HADES_MODULE       "Enable HADES SAF module."        ON)

# Add JUCE and Spatial_Audio_Framework to the project
add_subdirectory(extern) 
juce_set_vst2_sdk_path(${CMAKE_CURRENT_SOURCE_DIR}/extern/VST2_SDK)

# Add core+interface libraries
add_subdirectory(C)

# Configure plugin
add_subdirectory(audio_plugin)