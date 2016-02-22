cmake_minimum_required (VERSION 2.8)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")


set(WRATHION_MODULE_NAME wrathion_module_pdf)

file( GLOB component_SOURCES *.cpp )
add_library(${WRATHION_MODULE_NAME} SHARED ${component_SOURCES})
if(WIN32)
  target_link_libraries(${WRATHION_MODULE_NAME} wrathion_core poppler.dll)
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(${WRATHION_MODULE_NAME} wrathion_core poppler.so)
endif()

set_target_properties(${WRATHION_MODULE_NAME} PROPERTIES PREFIX "")

file(COPY kernels DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})