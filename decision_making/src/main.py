import rospy
from lar_msgs.msg import Map
import sys
TIMES = 15
cnt = 0
def map_callback(data):
  global cnt
  cnt += 1
  if not cnt == TIMES:
    return
  cnt = 0
  maps = []
  for i in range(data.info.width):
    v = []
    for j in range(data.info.height):
      val = data.data[i*data.info.width + j].height
      if val == -100:
	v.append(0)
      else:
	v.append(1)
    maps.append(v)
  for line in maps:
    for i,val in enumerate(line):
      if i>50 and i<101:
	system.
      
      
  
if __name__ == "__main__":
  rospy.init_node('a_star')
  rospy.Subscriber('/PER/Map',Map,map_callback)
  
  rospy.spin()