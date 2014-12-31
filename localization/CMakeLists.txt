cmake_minimum_required(VERSION 2.8.3)
project(localization)

find_package(catkin REQUIRED COMPONENTS
  message_generation
  cv_bridge
  roscpp
  cv_bridge
  lar_msgs
)
find_package(OpenCV 2)
find_package(Boost REQUIRED COMPONENTS system)


catkin_package(

)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)
add_executable(loc_node src/main.cpp src/ekf_class.cpp src/noiseless_estimator.cpp)
add_executable(user_function_example src/userHelperFunction/example.cpp)


target_link_libraries(loc_node ${catkin_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBS})
