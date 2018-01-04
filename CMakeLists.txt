cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(sdf_text)

# Some general project setup and helper functions moved to a separate file.
include(helpers.cmake)

#
# Set some helper variables.
#
set(projectDir  "${CMAKE_CURRENT_LIST_DIR}")
set(binDir      "${projectDir}/bin")

# Output executable name.
# NOTE: if you change this you also need to change your Android activity Java code accordingly.
set(targetName "sdf_text")

# Define executable output dir.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${binDir}/${targetSystem}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${binDir}/${targetSystem}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${binDir}/${targetSystem}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${binDir}/${targetSystem}_debug")


#
# App sources and includes (relative to the project root dir).
#
set(projectSources
  # Note: headers are only listed to show up in generated IDE projects.
  # Only the C-files need to be compiled.
  source/ok_gl_helper/ok_gl_helper.h
  source/ok_platform_wrapper/ok_app.h
  source/ok_platform_wrapper/ok_log.h
  source/ok_platform_wrapper/ok_opengl.h
  source/ok_platform_wrapper/ok_platform.h

  source/main.c
  source/sdf_text_app.c
)

set(projectIncludeDirs ${projectIncludeDirs}
  source
)

if(NOT MSVC)
  # Link to the c math library (not when using msvc).
  set(projectLinkLibs ${projectLinkLibs} m)
endif()

#
# Depencency: Fontstash (header only library).
#
set(projectIncludeDirs ${projectIncludeDirs} "external/fontstash/src")


#
# Depencency: GLEW (only when not using OpenGL ES).
#
if(NOT ANDROID AND NOT EMSCRIPTEN)
  add_subdirectory("external/glew-2.1.0/build/cmake" EXCLUDE_FROM_ALL)
  # Using static linking for now to remove one more lib to install.
  set(projectLinkLibs ${projectLinkLibs} glew_s)
  set(projectIncludeDirs ${projectIncludeDirs} "${projectDir}/external/glew-2.1.0/include")
endif()

#
# Depencency: SDL2.
#
if(TRUE)
  # Use prebuilt SDL binaries.
  add_subdirectory("external/SDL-prebuilt" EXCLUDE_FROM_ALL)
  set(projectLinkLibs ${projectLinkLibs} SDL2-prebuilt)
else()
  # Build SDL from source.
  # TODO: not tested for a while.
  # TODO: Emscripten requires static linking if using SDL sources.
  add_subdirectory("external/SDL" EXCLUDE_FROM_ALL)
  set(projectIncludeDirs ${projectIncludeDirs} "${projectDir}/external/SDL/include")
  set(projectLinkLibs ${projectLinkLibs} SDL2main SDL2)
endif()


#
# Configure the application executable. ${targetName} is also the default name
# for the output binary.
#
if(ANDROID)
  # On android the final binary is a shared library not an executable.
  add_library(${targetName} SHARED ${projectSources})
else()
  add_executable(${targetName} ${projectSources})
endif()

target_include_directories(${targetName} PUBLIC ${projectIncludeDirs})
target_link_libraries(${targetName} ${projectLinkLibs} )

# Using C99 standard.
set_property(TARGET ${targetName} PROPERTY C_STANDARD 99)

#
# This will automatically copy any predefined binary dependencies to the output dir
# (pretty much just here for windows dlls). It looks for a custom property in each
# target that list files that need to be copied.
#
target_copy_binary_dependencies(${targetName} "${projectLinkLibs}")

