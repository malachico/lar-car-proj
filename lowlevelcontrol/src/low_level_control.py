#!/usr/bin/env python
import rospy
from ros_interface import ROS
from geometry_msgs.msg import Point
from std_msgs.msg import String
import math
ros = ROS()
gas = 0

def set_gas(data):
  global gas
  gas = float(data.data)  
  
def get_wp(data):
    global gas
    d,theta = calc_error(ros.loc,[data.x,data.y])
    g,s = PID(d,theta) 
    ros.speed.set(g*3*gas)
    if math.isnan(ros.speed.get()):
       print "Warning: ros speed is NAN. Reset to 0."
       ros.speed.set(0)
    ros.steer.set(s)
    ros.publisher()
    
def calc_error(car_loc,WP):
    dist = float((WP[0]-car_loc['x'])**2+(WP[1]-car_loc['y'])**2)**0.5
    des = math.atan2(WP[1]-car_loc['y'], WP[0]-car_loc['x'])
    #print "desired: ",des," act: ", car_loc['theta']
    ori_error =  180.0/math.pi*(des-car_loc['theta'])
    if ori_error<-180:
        ori_error += 360
    if ori_error>180:
        ori_error -= 360
    return dist, ori_error
    
def PID(dist,dt):
    s = dt/45
    g = dist*0.01
    if g<0.05:
        g = 0.05
    if dist<1.0:
        g = 0
    return g,s

  
if __name__ == "__main__":
  print 'Starting Low level control...'
  rospy.Subscriber('/LPP/WP',Point,get_wp)
  rospy.Subscriber('/Drive',String,set_gas)
  print 'Node is Running. Will publish to /drivingVic topics'
  rospy.spin()
  print 'LLC: "Goodbye..."'