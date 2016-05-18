cmake_minimum_required (VERSION 2.8)

include_directories("${PROJECT_SOURCE_DIR}/include")
set(CMAKE_CXX_FLAGS "-pthread")

file( GLOB component_SOURCES *.cpp )
add_library(wrathion_core SHARED ${component_SOURCES})
set_target_properties(wrathion_core PROPERTIES PREFIX "")

if (WIN32)
  target_link_libraries(wrathion_core ws2_32)
endif (WIN32)

target_link_libraries(wrathion_core OpenCL)

file(COPY kernels DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
