#!/usr/bin/env python

from google_map_coordinates.srv import *
from sensor_msgs.msg import NavSatFix
import os
import subprocess
import rospy
import thread

loc = NavSatFix()
def run_django_server(data):
  subprocess.call(['python', os.path.dirname(os.path.realpath(__file__)) + '/../django/django_projects/mysite/manage.py','runserver'])

def gps_callback(data):
  global loc
  loc = data

def get_coordinates(req):
  global loc
  return CoordinatesResponse(loc)

def get_coordinates_server():
    rospy.init_node('get_coordinates_server')
    sub = rospy.Subscriber('/SENSORS/GPS', NavSatFix, gps_callback)
    s = rospy.Service('get_coordinates', Coordinates, get_coordinates)    
    print "Ready to send coordinates."
    rospy.spin()

if __name__ == "__main__":
  thread.start_new_thread(run_django_server,(None,))
  get_coordinates_server()
