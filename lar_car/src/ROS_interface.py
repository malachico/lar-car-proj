import rospy
from std_msgs.msg import Float64
from std_msgs.msg import Int8
from geometry_msgs.msg import PoseWithCovarianceStamped
from gazebo_msgs.msg import ModelStates
from sensor_msgs.msg import Image
from PIL import Image as Img
from PIL import ImageFilter,ImageTk
from tf.transformations import euler_from_quaternion
import numpy, math
import gtk

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

PX = 1288
PY =  964

class ROS:
    def __init__(self,gui):
	self.imageR_canvas = gui.imageR
	self.imageL_canvas = gui.imageL
	self.gui = gui
	#self.canvas = canvas
        self.speed = var(0,5)
        self.steer = var(-10,10)
        self.steerlog = []
        self.brake = var(0,1)
        self.hb = var()
        self.hb.set(0)
        self.direction = var()
        self.direction.set(1)
        self.loc = {'x':0.0,'y':0.0,'theta':0.0}
        self.real_loc = {'x':0.0,'y':0.0,'theta':0.0}        
        rospy.init_node('operator')
        #Publishers
        self._gas = rospy.Publisher('/drivingVic/gas_pedal/cmd', Float64)
        self._brake = rospy.Publisher('/drivingVic/brake_pedal/cmd', Float64)
        self._wheel = rospy.Publisher('/drivingVic/hand_wheel/cmd', Float64)
        self._hand_brake = rospy.Publisher('/drivingVic/hand_brake/cmd', Float64)
        self._direction = rospy.Publisher('/drivingVic/direction/cmd', Int8)
        self._key = rospy.Publisher('/drivingVic/key/cmd', Int8)
        #Subscriber
        self._loc = rospy.Subscriber('/LOC/Pose',PoseWithCovarianceStamped, self.location)
        self._real_loc = rospy.Subscriber('/gazebo/model_states',ModelStates, self.real_location)        
        self._image = rospy.Subscriber('/SENSORS/CAM/L', Image, self.imageR_callback)
        self._image = rospy.Subscriber('/SENSORS/CAM/R', Image, self.imageL_callback)
        self._steerHistory = rospy.Subscriber('/drivingVic/hand_wheel/state', Float64, self.log_steer)
        self.img = 0
        self.img_acquiered = False
        self.image_handler = 0
        

    def real_location(self,obj):
        i = 0
        for objecti in obj.name:
            if objecti == 'drivingVic':
                val = i
                break
            i += 1
        theta = euler_from_quaternion( numpy.array((obj.pose[val].orientation.x, obj.pose[val].orientation.y, obj.pose[val].orientation.z, obj.pose[val].orientation.w), dtype=numpy.float64))
        self.real_loc['x'] = int(obj.pose[val].position.x*100)/100.0
        self.real_loc['y'] = int(obj.pose[val].position.y*100)/100.0
        self.real_loc['theta'] = int(theta[2]*100)/100.0
        
    def location(self,obj):
        self.loc['x'] = obj.pose.pose.position.x
        self.loc['y'] = obj.pose.pose.position.y
        theta = euler_from_quaternion( numpy.array((obj.pose.pose.orientation.x, obj.pose.pose.orientation.y, obj.pose.pose.orientation.z, obj.pose.pose.orientation.w), dtype=numpy.float64))
        self.loc['theta'] = theta[2]
        self.gui.loc_txt.set_label('x:%.2f\nx:%.2f\nx:%.2f\n'%(self.loc['x'],self.loc['y'],self.loc['theta']))

    def imageR_callback(self, data):
	self.img = numpy.array(Img.frombuffer("RGB", [PX, PY], data.data, 'raw', "RGB", 0, 1))#.crop((0, PY,PX, PY))#.resize((40, 25)).convert('1')
	pix = gtk.gdk.pixbuf_new_from_array(self.img,gtk.gdk.COLORSPACE_RGB,8)
	pix = pix.scale_simple(320,240,gtk.gdk.INTERP_BILINEAR)
	self.imageR_canvas.set_from_pixbuf(pix)

    def imageL_callback(self, data):
	self.img = numpy.array(Img.frombuffer("RGB", [PX, PY], data.data, 'raw', "RGB", 0, 1))#.crop((0, PY,PX, PY))#.resize((40, 25)).convert('1')
	pix = gtk.gdk.pixbuf_new_from_array(self.img,gtk.gdk.COLORSPACE_RGB,8)
	pix = pix.scale_simple(320,240,gtk.gdk.INTERP_BILINEAR)
	self.imageL_canvas.set_from_pixbuf(pix)
	
    def log_steer(self,data):
        num = int(data.data*100)/100.0
        self.steerlog.append(num)
        if len(self.steerlog)>20:
            self.steerlog.pop(0)

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
