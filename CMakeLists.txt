cmake_minimum_required (VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
set(CMAKE_CXX_FLAGS "-pthread")

project (Wrathion)

include_directories("${PROJECT_SOURCE_DIR}/include")
#include_directories ("${PROJECT_SOURCE_DIR}/core")

add_definitions( -DWRATHION_DLL_MODULES )
add_definitions( -std=c++11 )
add_definitions( -DNDEBUG )
add_definitions( -march=native )
add_definitions( -fomit-frame-pointer )

SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_BUILD_TYPE Debug)

add_subdirectory(core)

add_executable(test core/Test.cpp)
add_dependencies(test wrathion_core)

target_link_libraries(test wrathion_core)
