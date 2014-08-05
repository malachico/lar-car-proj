import rospy
from std_msgs.msg import Float64
from std_msgs.msg import Int8
from geometry_msgs.msg import TwistStamped,PoseWithCovarianceStamped
from tf.transformations import euler_from_quaternion
import numpy, math

class var:
    def __init__(self,min_val=-100,max_val=100):
        self.var = 0
        self.max_val = max_val
        self.min_val = min_val
    def set(self, val):
        self.var = val
        self.fix_value()
    def get(self):
        return self.var
    def add(self,val):
	self.var += val
	self.fix_value()
    def fix_value(self):
      if self.var > self.max_val:
	self.var = self.max_val
      elif self.var < self.min_val:
	self.var = self.min_val
    def toggle(self):
        if self.var == 0:
            self.var = 1
        else:
            self.var = 0

class ROS:
    def __init__(self):
	self.speed = var(0,5)
        self.steer = var(-10,10)
        self.brake = var(0,1)
        self.hb = var()
        self.hb.set(0)
        self.direction = var()
        self.direction.set(1)
        self.loc = {'x':0.0,'y':0.0,'theta':0.0,'x_dot':0.0,'y_dot':0.0}
        rospy.init_node('low_level_control')
        #Publishers
        self._gas = rospy.Publisher('/drivingVic/gas_pedal/cmd', Float64)
        self._brake = rospy.Publisher('/drivingVic/brake_pedal/cmd', Float64)
        self._wheel = rospy.Publisher('/drivingVic/hand_wheel/cmd', Float64)
        self._hand_brake = rospy.Publisher('/drivingVic/hand_brake/cmd', Float64)
        self._direction = rospy.Publisher('/drivingVic/direction/cmd', Int8)
        self._key = rospy.Publisher('/drivingVic/key/cmd', Int8)
        #Subscriber
        self._loc = rospy.Subscriber('/LOC/Pose',PoseWithCovarianceStamped, self.location)
        self._speed = rospy.Subscriber('/LOC/Velocity',TwistStamped,self.velocity)
        

  
    def velocity(self,data):
	self.loc['x_dot'] = data.twist.linear.x
	self.loc['y_dot'] = data.twist.linear.y
	
    def location(self,obj):
        self.loc['x'] = obj.pose.pose.position.x
        self.loc['y'] = obj.pose.pose.position.y
        theta = euler_from_quaternion( numpy.array((obj.pose.pose.orientation.x, obj.pose.pose.orientation.y, obj.pose.pose.orientation.z, obj.pose.pose.orientation.w), dtype=numpy.float64))
        self.loc['theta'] = theta[2]

    def publisher(self):
        self.publish_int8()
        self.publish_float64()

    def publish_int8(self):
        msg = Int8()
        #publish direction
        msg.data = self.direction.get()
        self._direction.publish(msg)

    def publish_float64(self):
        msg = Float64()
        #publish HB
        msg.data = self.hb.get()
        self._hand_brake.publish(msg)
        #publish Gas
        msg.data = self.speed.get()
        self._gas.publish(msg)
        msg.data = self.brake.get()
        self._brake.publish(msg)
        #publish Steering
        msg.data = self.steer.get()
        self._wheel.publish(msg)

