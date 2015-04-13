import rospy
import rosbag
from std_msgs.msg import Int8, String

bag = rosbag.Bag('test.bag', 'w')

def get_dat(data):
  bag.write('/test',data)

def main():
  rospy.init_node('record_test')
  rospy.Subscriber('/test',Int8,get_dat)
  rospy.spin()
try:
    main()
finally:
    bag.close()