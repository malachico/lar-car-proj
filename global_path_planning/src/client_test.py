#!/usr/bin/env python

import sys
import rospy
from nav_msgs.srv import GetPlan
from geometry_msgs.msg import PoseStamped 
from url_search import *
from gps_calculator import *
init_pos = NavSatFix()
init_pos.latitude = 31.26
init_pos.longitude = 34.80

def get_directions(x, y):
    rospy.wait_for_service('/GPP')
    try:
        gpp = rospy.ServiceProxy('/GPP', GetPlan)
        resp1 = gpp(x, y,0)
        return resp1.plan
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

if __name__ == "__main__":
    address1 = 'HaZamir+12,Rishon+Lezion,+Israel'
    address2 = 'Kaplan+20,Quiriat+Ono,+Israel'
    start_p = address2geo(address1)
    goal_p = address2geo(address2)
    start = geo2xy(init_pos,start_p)
    goal = geo2xy(init_pos,goal_p)
    directions = get_directions(start,goal)
    #print directions[0]
    x = []
    y = []
    for d in directions.poses:
      x.append(d.pose.position.x)
      y.append(d.pose.position.y)
    import matplotlib.pyplot as plt
    plt.plot(x,y)
    plt.ylabel('some numbers')
    plt.show()
    