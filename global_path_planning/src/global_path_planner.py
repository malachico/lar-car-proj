#!/usr/bin/env python

import rospy
from sensor_msgs.msg import NavSatFix
from nav_msgs.srv import GetPlan
from nav_msgs.msg import Path
from geometry_msgs.msg import PoseStamped 
from url_search import *
from gps_calculator import *

def get_directions_service(req):
    orig = "31.26,34.80"#"%f,%f"%(req.start.pose.position.x,req.start.pose.position.y)
    dest = "Masarik 5, Bat Yam"#"%f,%f"%(req.goal.pose.position.x,req.goal.pose.position.y)
    init_pos = NavSatFix()
    init_pos.latitude = req.start.pose.position.x
    init_pos.longitude = req.start.pose.position.y
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