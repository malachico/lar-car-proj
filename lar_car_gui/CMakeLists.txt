cmake_minimum_required(VERSION 2.8.3)
project(lar_car_gui)

find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  roscpp
  stereo_package
)

find_package(OpenCV 2)
find_package(Boost REQUIRED COMPONENTS system)
catkin_package(
	  CATKIN_DEPENDS
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
# include_directories(include)
include_directories(
  ${catkin_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

## Declare a cpp library


## Declare a cpp executable
 add_executable(lar_car_gui_node src/main.cpp)

## Add cmake target dependencies of the executable/library
## as an example, message headers may need to be generated before nodes
 #add_dependencies(lar_car_gui_node lar_car_gui_generate_messages_cpp)

## Specify libraries to link a library or executable target against
 target_link_libraries(lar_car_gui_node
   ${catkin_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBS} 
 )