import rospy
from std_msgs.msg import Float64
from std_msgs.msg import Int8
from geometry_msgs.msg import PoseWithCovarianceStamped,Point
from gazebo_msgs.msg import ModelStates
from sensor_msgs.msg import Image
from PIL import Image as Img
from PIL import ImageFilter,ImageTk
from tf.transformations import euler_from_quaternion
import numpy, math
import gtk

COUNT_TIMES = 3
PX = 1288
PY =  964

class ROS:
  
    def __init__(self,gui):
      self.display_counter = {'loc':0,'wp':0}
      self.gui = gui
      self.loc = {'x':0.0,'y':0.0,'theta':0.0}      
      rospy.init_node('operator')        
      #Publishers
      gui.drive = rospy.Publisher('/Drive', Float64)
      #Subscriber
      self._loc = rospy.Subscriber('/LOC/Pose',PoseWithCovarianceStamped, self.location)
      self._loc = rospy.Subscriber('/LPP/WP',Point, self.get_wp)      
      self._image = rospy.Subscriber('/SENSORS/CAM/L', Image, self.imageR_callback)
      self._image = rospy.Subscriber('/SENSORS/CAM/R', Image, self.imageL_callback)
        
    def get_wp(self,data):
      if self.display_counter['wp'] == 0:
	self.gui.wp_txt.set_label('x: %.2f\ny: %.2f'%(data.x,data.y))
      elif self.display_counter['wp'] == COUNT_TIMES:
	self.display_counter['wp'] = -1
      self.display_counter['wp'] += 1
      
    def location(self,obj):
      if self.display_counter['loc'] == 0:
	self.loc['x'] = obj.pose.pose.position.x
	self.loc['y'] = obj.pose.pose.position.y
	theta = euler_from_quaternion( numpy.array((obj.pose.pose.orientation.x, obj.pose.pose.orientation.y, obj.pose.pose.orientation.z, obj.pose.pose.orientation.w), dtype=numpy.float64))
	self.loc['theta'] = theta[2]
	self.gui.loc_txt.set_label('x:%.2f\ny:%.2f\nY:%.2f\n'%(self.loc['x'],self.loc['y'],self.loc['theta']))
      elif self.display_counter['loc'] == COUNT_TIMES:
	self.display_counter['loc'] = -1
      self.display_counter['loc'] += 1
      
    def imageR_callback(self, data):
      self.img = numpy.array(Img.frombuffer("RGB", [PX, PY], data.data, 'raw', "RGB", 0, 1))#.crop((0, PY,PX, PY))#.resize((40, 25)).convert('1')
      pix = gtk.gdk.pixbuf_new_from_array(self.img,gtk.gdk.COLORSPACE_RGB,8)
      pix = pix.scale_simple(320,240,gtk.gdk.INTERP_BILINEAR)
      self.gui.imageR.set_from_pixbuf(pix)

    def imageL_callback(self, data):
      self.img = numpy.array(Img.frombuffer("RGB", [PX, PY], data.data, 'raw', "RGB", 0, 1))#.crop((0, PY,PX, PY))#.resize((40, 25)).convert('1')
      pix = gtk.gdk.pixbuf_new_from_array(self.img,gtk.gdk.COLORSPACE_RGB,8)
      pix = pix.scale_simple(320,240,gtk.gdk.INTERP_BILINEAR)
      self.gui.imageL.set_from_pixbuf(pix)