#!/usr/bin/env python
import rospy
from ros_interface import ROS
from geometry_msgs.msg import Point
from std_msgs.msg import Float64
from joystick_input_control import joystick_control
import math
ros_handler = ROS()
joy = joystick_control()
gas = 0
try:
  while True:
    but, ax = joy.get_all()
    ros_handler.speed.set(abs(ax[2]-1.0))
    ros_handler.steer.set(-ax[0])
    ros_handler.publisher()
    rospy.sleep(0.1)
  
finally:
  ros_handler.speed.set(0)
  ros_handler.steer.set(0)
  ros_handler.publisher()
  print 'goodbye'
  