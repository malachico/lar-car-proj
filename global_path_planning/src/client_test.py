#!/usr/bin/env python

import sys
import rospy
from nav_msgs.srv import GetPlan
from geometry_msgs.msg import PoseStamped 

def get_directions(x, y):
    rospy.wait_for_service('/GPP')
    try:
        gpp = rospy.ServiceProxy('/GPP', GetPlan)
        resp1 = gpp(x, y,0)
        return resp1.plan
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

if __name__ == "__main__":
    start = PoseStamped()
    start.pose.position.x = 31.26
    start.pose.position.y = 34.80
    
    goal = PoseStamped()
    goal.pose.position.x = 31.2
    goal.pose.position.y = 34.7
    
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
    