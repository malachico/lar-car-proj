#!/bin/bash 

xterm -e "rosrun localization loc_node" &
xterm -e "rosrun lowlevelcontrol low_level_control.py" &
xterm -e "rosrun local_path_planning path_planning.py" &
xterm -e "rosrun stereo_package stereo_package" &
xterm -e "rosrun road_detection listener" &
