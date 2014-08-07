from django.shortcuts import render
from django.http import HttpResponse
import json
# from file import oded's_code..
import random 
import rospy
from google_map_coordinates.srv import *
from sensor_msgs.msg import NavSatFix

def get_coordinates_client():
    print 'waitting for service'
    rospy.wait_for_service('get_coordinates')
    try:
        get_coordinates = rospy.ServiceProxy('get_coordinates', Coordinates)
        resp1 = get_coordinates()
        return resp1.coordinates
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e
        
def index(request):
  print 'Asking for coordinates'
  coordinates = get_coordinates_client() 
  x= coordinates.latitude 
  y = coordinates.longitude

  res = HttpResponse( json.dumps({"x": x, "y": y}) )
  res['Access-Control-Allow-Origin'] = "*"
  return res
