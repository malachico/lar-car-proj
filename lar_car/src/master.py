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
ros_handler = ROS(gui_handler.imageR,gui_handler.imageL)
#def keydown(key):
    #global controller, SPEED, STEER,frame
    #if key == simplegui.KEY_MAP['space']:
        #STEER= 0
        #controller.steer.set(STEER)
    #if key == simplegui.KEY_MAP['e']:
        #SPEED = 1        
    #if key == simplegui.KEY_MAP['d']:
       #SPEED = -1
    #if key == simplegui.KEY_MAP['m']:
        #SPEED = 666            
    #if key == simplegui.KEY_MAP['x']:
        #SPEED = 0
        #controller.speed.set(SPEED)
        #controller.brake.set(10)
    #if key == simplegui.KEY_MAP['s']:
        #STEER = 1
    #if key == simplegui.KEY_MAP['f']:
        #STEER = -1
    #if key == simplegui.KEY_MAP['a']:
        #STEER = 10
    #if key == simplegui.KEY_MAP['g']:
        #STEER = -10        
    #if key == simplegui.KEY_MAP['down']:
        #controller.hb.toggle()
    #if key == simplegui.KEY_MAP['up']:
        #controller.direction.set(1) 
    #if key == simplegui.KEY_MAP['p']:
            #to_file()
    #if key == simplegui.KEY_MAP['q']:
        #print "goodbye"
#def keyup(key):
    #global controller, SPEED, STEER
    #if key == simplegui.KEY_MAP['e']:
        #SPEED = 0
        #pass#controller.speed.set(0)   
    #if key == simplegui.KEY_MAP['d']:
        #SPEED = 0
        #pass#controller.brake.set(0)
    #if key == simplegui.KEY_MAP['x']:
        #controller.brake.set(0)
    #if key == simplegui.KEY_MAP['s']:
        #controller.img._image.save("image_bw","png")
        #pass#controller.steer.set(0)   
    #if key == simplegui.KEY_MAP['f']:
        #STEER = 0
    #if key == simplegui.KEY_MAP['a']:
        #STEER = 0
        #pass#controller.steer.set(0)   
    #if key == simplegui.KEY_MAP['g']:
        #STEER = 0        
        #pass#controller.steer.set(0)        
    #if key == simplegui.KEY_MAP['down']:
        #pass
    #if key == simplegui.KEY_MAP['up']:
        #pass

gui_handler.run_main()