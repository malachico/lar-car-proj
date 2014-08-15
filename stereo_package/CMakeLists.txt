
cmake_minimum_required(VERSION 2.8.3)
project(stereo_package)
find_package(catkin REQUIRED COMPONENTS
    message_generation
    roscpp rospy
    cv_bridge
    std_msgs
    nav_msgs
#     robil2conf
    gazebo_ros
#     decision_making decision_making_parser
)
find_package(OpenCV 2)
find_package(Boost REQUIRED COMPONENTS system)
find_package(gazebo REQUIRED)


add_message_files(
    DIRECTORY msg
    FILES
        MapCell.msg
        Map.msg
)

generate_messages(
  DEPENDENCIES
  std_msgs
  nav_msgs
  )


catkin_package(
	CATKIN_DEPENDS
# 	    robil2conf
	    gazebo_ros
	    message_runtime
# 	    decision_making decision_making_parser
)

###########
## Build ##
###########
link_directories(${GAZEBO_LIBRARY_DIRS})
include_directories(
  ${catkin_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${GAZEBO_INCLUDE_DIRS}
)
## Declare a cpp library

link_directories(${GAZEBO_LIBRARY_DIRS})
add_executable(stereo_package src/main.cpp src/helpermath.cpp src/heightmap.cpp )
target_link_libraries(stereo_package ${catkin_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBS} ${GAZEBO_libraries} 
)
link_directories(${GAZEBO_LIBRARY_DIRS})
