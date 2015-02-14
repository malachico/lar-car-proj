#!/usr/bin/env python

import rospy
from sensor_msgs.msg import NavSatFix
from nav_msgs.srv import GetPlan
from nav_msgs.msg import Path
from geometry_msgs.msg import PoseStamped 
from url_search import *
from gps_calculator import *

init_pos = NavSatFix()
init_pos.latitude = 31.26
init_pos.longitude = 34.80

def get_directions_service(req):
    p = xy2geo(init_pos,req.start)
    orig = "%f,%f"%(p.latitude,p.longitude)
    p = xy2geo(init_pos,req.goal)
    dest = "%f,%f"%(p.latitude,p.longitude)
    print "From ", orig
    print "To ", dest
    print "--------"
    direction_list,status = get_directions(orig,dest)
    #print status
    res = Path()
    for wp in direction_list:
      point = NavSatFix()
      point.latitude = float(wp.start_location[0])
      point.longitude = float(wp.start_location[1])
      pos = geo2xy(init_pos,point)
      res.poses.append(pos)
    return res

def google_directions_server():
    rospy.init_node('google_directions_server')
    s = rospy.Service('/GPP', GetPlan, get_directions_service)
    print "Ready to generate direction"
    rospy.spin()

if __name__ == "__main__":
    google_directions_server()