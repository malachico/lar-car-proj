cmake_minimum_required(VERSION 2.8.3)
project(lar_car)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
geometry_msgs
  rospy
  std_msgs
  sensor_msgs
)

###################################
## catkin specific configuration ##
###################################
catkin_package(

)

###########
## Build ##
###########
include_directories(
  ${catkin_INCLUDE_DIRS}
)


#############
## Install ##
#############
catkin_install_python(PROGRAMS src/master.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
install(
  DIRECTORY models/
  DESTINATION $ENV{HOME}/.gazebo/models
)
install(
  DIRECTORY plugins/
  DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}/../../devel/lib
)
install(
  DIRECTORY ../scripts/
  DESTINATION $ENV{HOME}/.ros/scripts
  PATTERN "../scripts/*"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                      GROUP_EXECUTE GROUP_READ)