cmake_minimum_required(VERSION 3.1)
project(SDL2-prebuilt C)
set(currentDir "${CMAKE_CURRENT_LIST_DIR}")

#
# Configure prebuilt SDL2 for different environments.
#
if(WIN32) 
  # For windows download and unpack the official precompiled SDL binaries.

  if(MSVC)
    set(SDL2.name "SDL2-2.0.7") # NOTE: This must match the dir name of what comes out of the archive.
    set(SDL2.archives.dir "${currentDir}/archives/msvc")
    set(SDL2.unpacked.dir "${CMAKE_BINARY_DIR}/unpacked/msvc")
    set(SDL2.filename "SDL2-devel-2.0.7-VC.zip")
    set(SDL2.uri "https://libsdl.org/release/${SDL2.filename}")

    # Check for 64 or 32 bit
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(libPrefix "x64")
    else()
      set(libPrefix "x86")
    endif()

    set(SDL2.archive.file "${SDL2.archives.dir}/${SDL2.filename}")
    set(SDL2.unpacked.toplevel.dir "${SDL2.unpacked.dir}/${SDL2.name}")

    # Decompress the archive to the build dir if not already done.
    if(NOT EXISTS "${SDL2.unpacked.toplevel.dir}")
      # Download the prebuilt binary if not found.
      if(NOT EXISTS "${SDL2.archive.file}")
        message("Downloading SDL2 to '${SDL2.archive.file}'")
        file(DOWNLOAD ${SDL2.uri} ${SDL2.archive.file} SHOW_PROGRESS)
      endif()

      message("Extracting SDL2 to '${SDL2.unpacked.dir}'")
      file(MAKE_DIRECTORY "${SDL2.unpacked.dir}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${SDL2.archive.file} WORKING_DIRECTORY "${SDL2.unpacked.dir}")
    endif()

    add_library(SDL2main STATIC IMPORTED GLOBAL)
    set_target_properties(SDL2main PROPERTIES
        IMPORTED_LOCATION "${SDL2.unpacked.toplevel.dir}/lib/${libPrefix}/SDL2main.lib"
    )

    add_library(SDL2 SHARED IMPORTED GLOBAL)
    set_target_properties(SDL2 PROPERTIES
        IMPORTED_LOCATION "${SDL2.unpacked.toplevel.dir}/lib/${libPrefix}/SDL2.dll"
        IMPORTED_IMPLIB "${SDL2.unpacked.toplevel.dir}/lib/${libPrefix}/SDL2.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2.unpacked.toplevel.dir}/include"
    )

    # Define a list of dependency files to be copied to the runtime output dir.
    set(binaryDependencies
       $<TARGET_FILE:SDL2> # the SDL.dll file.
    )

    add_library(SDL2-prebuilt INTERFACE)
    target_link_libraries(SDL2-prebuilt
        INTERFACE SDL2main
        INTERFACE SDL2)
    set_target_properties(SDL2-prebuilt PROPERTIES
        INTERFACE_COPY_BINARY_DEPENDENCIES ${binaryDependencies}
    )

  elseif(MINGW)
    set(SDL2.name "SDL2-2.0.7") # NOTE: This must match the dir name of what comes out of the archive.
    set(SDL2.archives.dir "${currentDir}/archives/mingw")
    set(SDL2.unpacked.dir "${CMAKE_BINARY_DIR}/unpacked/mingw")
    set(SDL2.filename "SDL2-devel-2.0.7-mingw.tar.gz")
    set(SDL2.uri "https://libsdl.org/release/${SDL2.filename}")

    # Check for 64 or 32 bit
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(libPrefix "x86_64-w64-mingw32")
    else()
      set(libPrefix "i686-w64-mingw32")
    endif()

    set(SDL2.archive.file "${SDL2.archives.dir}/${SDL2.filename}")
    set(SDL2.unpacked.toplevel.dir "${SDL2.unpacked.dir}/${SDL2.name}")

    # Decompress the archive to the build dir if not already done.
    if(NOT EXISTS "${SDL2.unpacked.toplevel.dir}")
      # Download the prebuilt binary if not found.
      if(NOT EXISTS "${SDL2.archive.file}")
        message("Downloading SDL2 to '${SDL2.archive.file}'")
        file(DOWNLOAD ${SDL2.uri} ${SDL2.archive.file} SHOW_PROGRESS)
      endif()

      message("Extracting SDL2 to '${SDL2.unpacked.dir}'")
      file(MAKE_DIRECTORY "${SDL2.unpacked.dir}")
      execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${SDL2.archive.file} WORKING_DIRECTORY "${SDL2.unpacked.dir}")
    endif()

    add_library(SDL2main STATIC IMPORTED GLOBAL)
    set_target_properties(SDL2main PROPERTIES
        IMPORTED_LOCATION "${SDL2.unpacked.toplevel.dir}/${libPrefix}/lib/libSDL2main.a"
    )

    add_library(SDL2 SHARED IMPORTED GLOBAL)
    set_target_properties(SDL2 PROPERTIES
        IMPORTED_LOCATION "${SDL2.unpacked.toplevel.dir}/${libPrefix}/bin/SDL2.dll"
        IMPORTED_IMPLIB "${SDL2.unpacked.toplevel.dir}/${libPrefix}/lib/libSDL2.dll.a"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2.unpacked.toplevel.dir}/${libPrefix}/include/SDL2"
    )

    # Define a list of dependency files to be copied to the runtime output dir.
    set(binaryDependencies
       $<TARGET_FILE:SDL2> # the SDL.dll file.
    )

    add_library(SDL2-prebuilt INTERFACE)
    target_link_libraries(SDL2-prebuilt
        INTERFACE SDL2main
        INTERFACE SDL2)
    set_target_properties(SDL2-prebuilt PROPERTIES
        INTERFACE_COPY_BINARY_DEPENDENCIES ${binaryDependencies}
    )

  else()
    message(FATAL_ERROR "win32 environment not recognized.")
  endif()


elseif(ANDROID)
  # Select SDL2 libs path according to the build type.
  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SDL_LIBS_DIR ${SDL_LIBS_DEBUG_DIR})
    set(ignoreMeDir ${SDL_LIBS_RELEASE_DIR}) #suppress warning
  else()
    set(SDL_LIBS_DIR ${SDL_LIBS_RELEASE_DIR})
    set(ignoreMeDir ${SDL_LIBS_DEBUG_DIR}) #suppress warning
  endif()
  message("SDL: include=${SDL_INCLUDE_DIR} libs=${SDL_LIBS_DIR}")

  # NOTE: There is currently no official way to include headers in an AAR
  # package or a way to reference native libraries in AAR from cmake.
  # So we have to configure the library manually.
  add_library(SDL2 SHARED IMPORTED GLOBAL)
  set_target_properties(SDL2 PROPERTIES
      IMPORTED_LOCATION "${SDL_LIBS_DIR}/${ANDROID_ABI}/libSDL2.so"
      INTERFACE_INCLUDE_DIRECTORIES "${SDL_INCLUDE_DIR}"
  )

  #
  # Find general android native library dependencies.
  #
  find_library(log-lib log)
  find_library(android-lib android)
  find_library(gles1-lib GLESv1_CM)
  find_library(gles2-lib GLESv2)

  #
  # A wrapper library interface.
  #
  add_library(SDL2-prebuilt INTERFACE)
  target_link_libraries(SDL2-prebuilt
      INTERFACE ${log-lib}
      INTERFACE ${android-lib}
      INTERFACE ${gles1-lib}
      INTERFACE ${gles2-lib}
      INTERFACE SDL2
  )


elseif(EMSCRIPTEN)
  # Just adding an empty library. No need to actually define libraries to link against with emscripten.
  add_library(SDL2-prebuilt INTERFACE)

  # Use SDL2 instead of the default SDL 1.2
  set(CMAKE_CXX_FLAGS "${SDL_CXX_FLAGS} -s USE_SDL=2" PARENT_SCOPE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2" PARENT_SCOPE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_SDL=2 -s FULL_ES2=1" PARENT_SCOPE)


elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # Using a preinstalled SDL2-dev package in your distribution package manager.
  find_package(PkgConfig REQUIRED)
  pkg_search_module(SDL2 REQUIRED sdl2)
  message("lib SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
  message("lib SDL2_LIBRARY_DIRS: ${SDL2_LIBRARY_DIRS}")
  message("lib SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
  message("lib SDL2_LDFLAGS: ${SDL2_LDFLAGS}")
  message("lib SDL2_LDFLAGS_OTHER: ${SDL2_LDFLAGS_OTHER}")
  message("lib SDL2_CFLAGS: ${SDL2_CFLAGS}")
  message("lib SDL2_CFLAGS_OTHER: ${SDL2_CFLAGS_OTHER}")

  add_library(SDL2-prebuilt INTERFACE)
  set_target_properties(SDL2-prebuilt PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}"
      INTERFACE_COMPILE_OPTIONS "${SDL2_CFLAGS_OTHER}"
  )
endif()

