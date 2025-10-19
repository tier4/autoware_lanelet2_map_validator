cmake_minimum_required(VERSION 3.14)
project(autoware_lanelet2_map_validator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF)

# Dependencies
find_package(ament_cmake REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(nlohmann_json REQUIRED)

# Extract package version
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.xml" PACKAGE_XML_CONTENT)
string(REGEX MATCH "<version>[^\n\r<]*</version>" VERSION_TAG "${PACKAGE_XML_CONTENT}")
string(REGEX REPLACE ".*<version>([^\n\r<]*)</version>.*" "\\1" PACKAGE_VERSION "${VERSION_TAG}")

set(PACKAGE_VERSION_STR "${PACKAGE_VERSION}")

message(STATUS "Building version ${PACKAGE_VERSION}")

# Default parameters to embed
file(READ "config/params.yaml" EMBEDDED_YAML)
file(READ "config/issues_info.json" EMBEDDED_JSON)

set(DEFAULT_YAML "${EMBEDDED_YAML}")
set(DEFAULT_JSON "${EMBEDDED_JSON}")

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/src/include/lanelet2_map_validator/embedded_defaults.hpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/lanelet2_map_validator/embedded_defaults.hpp"
  @ONLY
)

# Source files
file(GLOB_RECURSE MAP_VALIDATOR_SRC
  src/common/*.cpp
  src/validators/*.cpp
)

# NOTE: keep this order in target_include_directories or the build system
# selects the wrong embedded_defaults.hpp
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
add_library(map_validator_includes INTERFACE)
target_include_directories(map_validator_includes INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
  /home/autoware_lanelet2_extension/autoware_lanelet2_extension/include
  /home/lanelet2/install/include
  ${EIGEN3_INCLUDE_DIR}
)

add_library(autoware_lanelet2_map_validator_obj OBJECT ${MAP_VALIDATOR_SRC})
target_link_libraries(autoware_lanelet2_map_validator_obj map_validator_includes)

#ament_target_dependencies(autoware_lanelet2_map_validator_obj
#  tf2
#  tf2_geometry_msgs
#  geometry_msgs
#  visualization_msgs
#)

# ========= Executable =============

add_executable(autoware_lanelet2_map_validator
  src/main.cpp
  $<TARGET_OBJECTS:autoware_lanelet2_map_validator_obj>
  /home/autoware_lanelet2_extension/autoware_lanelet2_extension/include/autoware_lanelet2_extension/projection/transverse_mercator_projector.hpp
  /home/autoware_lanelet2_extension/autoware_lanelet2_extension/include/autoware_lanelet2_extension/projection/mgrs_projector.hpp
)

# ========= Link Libraries =========

target_link_libraries(autoware_lanelet2_map_validator map_validator_includes)

#ament_target_dependencies(autoware_lanelet2_map_validator
#  tf2
#  tf2_geometry_msgs
#  geometry_msgs
#  visualization_msgs
#)

target_link_options(autoware_lanelet2_map_validator PRIVATE -Wl,--start-group)

target_link_libraries(autoware_lanelet2_map_validator
  /home/autoware_lanelet2_extension/build/libautoware_lanelet2_extension_lib.a

  /home/lanelet2/install/lib/liblanelet2_validation.a
  /home/lanelet2/install/lib/liblanelet2_routing.a
  /home/lanelet2/install/lib/liblanelet2_traffic_rules.a
  /home/lanelet2/install/lib/liblanelet2_io.a
  /home/lanelet2/install/lib/liblanelet2_projection.a
  /home/lanelet2/install/lib/liblanelet2_core.a

  /usr/lib/x86_64-linux-gnu/libyaml-cpp.a
  /home/fmt/install/lib/libfmt.a
  nlohmann_json::nlohmann_json
  /usr/lib/x86_64-linux-gnu/libboost_program_options.a
  /usr/lib/x86_64-linux-gnu/libboost_filesystem.a
  /usr/lib/x86_64-linux-gnu/libboost_system.a

  /usr/lib/x86_64-linux-gnu/libboost_serialization.a
  /usr/lib/x86_64-linux-gnu/libboost_wserialization.a

  /usr/lib/x86_64-linux-gnu/libGeographic.a
  /usr/lib/x86_64-linux-gnu/libpugixml.a

  stdc++fs
)

target_link_options(autoware_lanelet2_map_validator PRIVATE -Wl,--end-group)

# ========= Install =========
install(TARGETS autoware_lanelet2_map_validator DESTINATION lib/${PROJECT_NAME})
install(PROGRAMS template/create_new_validator.py DESTINATION lib/${PROJECT_NAME})

# Optional config files
install(
  DIRECTORY config map_requirements
  DESTINATION share/${PROJECT_NAME}
)

# Optional test data if desired
install(DIRECTORY test/data DESTINATION share/${PROJECT_NAME})

ament_package()
