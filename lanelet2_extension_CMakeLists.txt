cmake_minimum_required(VERSION 3.14)
project(autoware_lanelet2_extension)

set(LANELET2_INSTALL_DIR "" CACHE PATH "Path to the custom Lanelet2 install directory")
if(NOT LANELET2_INSTALL_DIR)
  message(FATAL_ERROR "LANELET2_INSTALL_DIR is not set! Please provide it via -D.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(autoware_map_msgs REQUIRED)
find_package(autoware_planning_msgs REQUIRED)

add_library(${PROJECT_NAME}_lib STATIC
  lib/autoware_osm_parser.cpp
  lib/autoware_traffic_light.cpp
  lib/crosswalk.cpp
  lib/detection_area.cpp
  lib/landmark.cpp
  lib/no_parking_area.cpp
  lib/no_stopping_area.cpp
  lib/bus_stop_area.cpp
  lib/message_conversion.cpp
  lib/mgrs_projector.cpp
  lib/query.cpp
  lib/road_marking.cpp
  lib/speed_bump.cpp
  lib/transverse_mercator_projector.cpp
  lib/utilities.cpp
  lib/virtual_traffic_light.cpp
  lib/visualization.cpp
  lib/route_checker.cpp
)

target_include_directories(${PROJECT_NAME}_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  "${LANELET2_INSTALL_DIR}/include"
  /usr/include/eigen3
)

ament_target_dependencies(${PROJECT_NAME}_lib
  rclcpp
  rcl_interfaces
  tf2
  tf2_geometry_msgs
  geometry_msgs
  visualization_msgs
  autoware_map_msgs
  autoware_planning_msgs
)

target_link_libraries(${PROJECT_NAME}_lib
  "${LANELET2_INSTALL_DIR}/lib/liblanelet2_core.a"
  "${LANELET2_INSTALL_DIR}/lib/liblanelet2_io.a"
  "${LANELET2_INSTALL_DIR}/lib/liblanelet2_projection.a"
  "${LANELET2_INSTALL_DIR}/lib/liblanelet2_routing.a"
  "${LANELET2_INSTALL_DIR}/lib/liblanelet2_traffic_rules.a"
  "${LANELET2_INSTALL_DIR}/lib/liblanelet2_validation.a"
)

install(TARGETS
  ${PROJECT_NAME}_lib
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/ DESTINATION include)

ament_package()
