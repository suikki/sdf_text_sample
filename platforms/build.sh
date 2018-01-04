#!/bin/sh

#
# Alternatively you can do something like this in the project root dir:
#   >mkdir build/make_debug
#   >cd build/make_debug
#   >cmake -DCMAKE_BUILD_TYPE:STRING=Debug ../..
#   >make
#

current_dir=$(cd -P -- "$(dirname -- "$0")" && pwd -P) || exit 1
source_dir="${current_dir}/.."
build_dir="${current_dir}/../build/make"


# Build debug version
cmake_options_debug="-DCMAKE_BUILD_TYPE:STRING=Debug"
cmake -E make_directory "${build_dir}_debug" || exit 1
cmake -E chdir "${build_dir}_debug" cmake $cmake_options_debug "$source_dir" || exit 1
cmake -E chdir "${build_dir}_debug" cmake --build . || exit 1

# Build Release version
cmake_options="-DCMAKE_BUILD_TYPE:STRING=Release"
cmake -E make_directory "${build_dir}" || exit 1
cmake -E chdir "${build_dir}" cmake $cmake_options "$source_dir" || exit 1
cmake -E chdir "${build_dir}" cmake --build . || exit 1
