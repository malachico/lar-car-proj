cmake_minimum_required(VERSION 2.8.3)
project(lar_msgs)


find_package(catkin REQUIRED COMPONENTS
message_generation
roscpp
std_msgs
geometry_msgs
nav_msgs
sensor_msgs
tf
)


## Generate messages in the 'msg' folder
add_message_files(
   FILES
   GpsSpeed.msg
   Map.msg
MapCell.msg
 )

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
generate_messages(
DEPENDENCIES
std_msgs
geometry_msgs
nav_msgs
sensor_msgs
)

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
# INCLUDE_DIRS include
# LIBRARIES robil_msgs
CATKIN_DEPENDS message_runtime geometry_msgs nav_msgs roscpp sensor_msgs std_msgs tf
# DEPENDS system_lib
)


