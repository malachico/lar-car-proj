#!/bin/bash 

rosrun google_map_coordinates master.py &
xterm -e "rosrun google_map_coordinates coor_service.py" &
