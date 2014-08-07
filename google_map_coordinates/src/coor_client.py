#!/usr/bin/env python

import sys
import rospy
from google_map_coordinates.srv import *

def get_coordinates_client():
    rospy.wait_for_service('get_coordinates')
    try:
        get_coordinates = rospy.ServiceProxy('get_coordinates', Coordinates)
        resp1 = get_coordinates()
        return resp1
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

if __name__ == "__main__":
    print get_coordinates_client()
