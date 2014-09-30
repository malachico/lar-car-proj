
cmake_minimum_required(VERSION 2.8.3)
project(lar_car_gui)
find_package(catkin REQUIRED COMPONENTS
    message_generation
    stereo_package
    roscpp rospy
    cv_bridge
    std_msgs
    nav_msgs
#     robil2conf
    gazebo_ros
#     lar_car_gui lar_car_gui_parser
)
find_package(OpenCV 2)
find_package(Boost REQUIRED COMPONENTS system)
find_package(gazebo REQUIRED)



catkin_package(
	CATKIN_DEPENDS
# 	    robil2conf
            stereo_package
	    gazebo_ros
	    message_runtime
# 	    lar_car_gui lar_car_gui_parser
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
add_executable(lar_car_gui src/main.cpp src/helpermath.cpp src/heightmap.cpp src/SimpleGUI.cpp)
target_link_libraries(lar_car_gui ${catkin_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBS} ${GAZEBO_libraries} 
)
link_directories(${GAZEBO_LIBRARY_DIRS})
