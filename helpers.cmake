
if(__project_setup)
	return()
endif()
set(__project_setup YES)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Please create a separate build directory and call cmake from there.")
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING
		"Choose the type of build, options are: Debug, RelWithDebInfo, Release, MinSizeRel"
		FORCE)
	message("CMAKE_BUILD_TYPE was not specified. Defaulting to ${CMAKE_BUILD_TYPE}")
endif()


if(CMAKE_BUILD_TYPE MATCHES Debug)
	message("Generating DEBUG version.")
	add_definitions(-DDEBUG)
	add_definitions(-D_DEBUG)

elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
	message("Generating RELEASE version (with debug info).")
	add_definitions(-DNDEBUG)

elseif(CMAKE_BUILD_TYPE MATCHES Release)
	message("Generating RELEASE version.")
	add_definitions(-DNDEBUG)

elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
	message("Generating RELEASE min size version.")
	add_definitions(-DNDEBUG)
endif()


#
# Platform specific options.
#

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")

  if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1")
  endif()

  # Embedding and mapping files at from_path@to_path
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ../../assets@assets")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s TOTAL_MEMORY=67108864")

  # Use webassembly
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1")

  # Use a custom html file template.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ../../platforms/emscripten/html/shell.html")
endif()

if(MINGW)
  # Linking standard libs statically to reduce dll clutter.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libstdc++")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -static-libgcc")

  # -lmingw32 is needed to make WinMain not disappear.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lmingw32")

  # Only show the console in debug builds and also strip unused dependencies on MinSizeRel.
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-mwindows")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-mwindows -s")

  # Uncomment to force console on release builds too.
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")
endif()


#
# Create a nicer name for the current target platform.
#
function(get_target_system)
  if(EMSCRIPTEN)
    set(targetSystem "web" PARENT_SCOPE)
    return()
  endif()

  # Check for 64 or 32 bit build target
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(targetBits "64")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(targetBits "32")
  else()
    message(FATAL_ERROR "Please create a separate build directory and call cmake from there.")
  endif()

  string(TOLOWER "${CMAKE_SYSTEM_NAME}" targetSystemBase)
  set(targetSystem ${targetSystemBase}${targetBits} PARENT_SCOPE)
endfunction()

get_target_system()

#
# Helper function to copy any binary dependencies (in practice windows dlls) to the output dir.
# Given a list of cmake targets this looks for a property called INTERFACE_COPY_BINARY_DEPENDENCIES
# on each dependency. Copies all files listed in the property to the target dir of the outputTarget.
#
function(target_copy_binary_dependencies outputTarget dependencies)
  foreach(dependencyTarget ${dependencies})
    if(TARGET ${dependencyTarget})
      get_target_property(binaryDependencies ${dependencyTarget} INTERFACE_COPY_BINARY_DEPENDENCIES)
      if(binaryDependencies)
        foreach(dependencyFile ${binaryDependencies})
          add_custom_command(TARGET ${outputTarget} PRE_BUILD
            #COMMAND ${CMAKE_COMMAND} -E echo "Copy: ${binaryDependencies} -> OUTPUTDIR($<CONFIG>): $<TARGET_FILE_DIR:${outputTarget}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dependencyFile}" "$<TARGET_FILE_DIR:${outputTarget}>/"
          )
        endforeach()
      endif()
    endif()
  endforeach()
endfunction()