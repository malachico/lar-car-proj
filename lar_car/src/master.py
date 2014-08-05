#!/usr/bin/env python

from GUI import car_gui
from ROS_interface import ROS
import thread,time
from PIL import ImageTk
import pygtk
import gtk,webkit

def keydown(widget, event):
  global ros_handler
  key = gtk.gdk.keyval_name(event.keyval)
  #print "Key %s (%d) was pressed" % (key, event.keyval)
  if key == 'KP_0':
    ros_handler.speed.set(0)
    ros_handler.steer.set(0)
  if key == 'Up':
    ros_handler.speed.add(0.1)
  if key == 'Down':
    ros_handler.speed.add(-0.1)
  if key == 'Left':
    ros_handler.steer.add(0.1)
  if key == 'Right':
    ros_handler.steer.add(-0.1)
  ros_handler.publisher()
gui_handler = car_gui((800,600),keydown)
ros_handler = ROS(gui_handler)

gui_handler.run_main()