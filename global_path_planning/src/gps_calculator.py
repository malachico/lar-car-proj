#!/usr/bin/env python
import sys
import rospy
from sensor_msgs.msg import NavSatFix
from geometry_msgs.msg import PoseStamped 
from math import atan2, sin, cos, sqrt, asin
PI  = 3.141592653589793238463
def calc_distance(init_pos,point):
  '''
    * calc_distance calculates the distance between two latitude/longitude points.
    * The equations are taken from the following site:
    * http://www.movable-type.co.uk/scripts/latlong.html
  '''
  R = 6371.0
  lat1 = init_pos.latitude*PI/180.0
  lat2 = point.latitude*PI/180.0
  dLat = lat2 - lat1
  dLon = (point.longitude - init_pos.longitude)*PI/180.0

  a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2)
  c = 2 * atan2(sqrt(a), sqrt(1-a))
  d = R*c

  return d*1000
  
def calc_bearing(init_pos,point):
    '''
      * calc the bearing from my initial position
    '''
    lat1 = init_pos.latitude*PI/180
    lat2 = point.latitude*PI/180
    dLat = lat2 - lat1
    dLon = (point.longitude - init_pos.longitude)*PI/180
    y = sin(dLon) * cos(lat2)
    x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon)
    brng = atan2(y, x)
    return brng

def calc_lat_long(init_pos,distance,brng):
  '''
    * calculate the latitude and longitude given distance and bearing
    * distnace input in [m] brng in [rad]
  '''
  R = 6371.0 * 1000
  p2 = NavSatFix()
  lat1 = init_pos.latitude*PI/180.0
  lon1 = init_pos.longitude*PI/180.0
	  
  p2.latitude = (asin( sin(lat1)*cos(distance/R) + cos(lat1)*sin(distance/R)*cos(brng)))
  p2.longitude = (lon1 + atan2(sin(brng)*sin(distance/R)*cos(lat1),cos(distance/R)-sin(lat1)*sin(p2.latitude)))*180 / PI
  p2.latitude = p2.latitude *180 / PI

  return p2

def geo2xy(init_pos,point):
    var = PoseStamped()
    d = calc_distance(init_pos,point)
    theta = calc_bearing(init_pos,point)
    var.pose.position.x = d * cos(theta)
    var.pose.position.y = d * sin(theta)
    return var

def xy2geo(init_pos,pos):
    point = pos.pose.position
    d = sqrt(point.x * point.x + point.y * point.y);
    theta = atan2(point.y,point.x);
    var = calc_lat_long(init_pos,d,theta);
    return var
    
if __name__ == "__main__":
  init_pos = NavSatFix()
  init_pos.latitude = 31.266
  init_pos.longitude = 34.80
  
  point = PoseStamped()
  point.pose.position.x = 100000
  point.pose.position.y = 100000
  
  pose_of_point = xy2geo(init_pos,point)
  print pose_of_point
  
  print "------------------"
  
  point_of_pose = geo2xy(init_pos,pose_of_point)
  print point_of_pose
  
  
  
  