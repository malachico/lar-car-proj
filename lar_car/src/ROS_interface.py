import rospy
from stereo_package.msg import Map
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
import string
COUNT_TIMES = 6
PX = 1288
PY =  964

class ROS:
  
    def __init__(self,gui):
      
      self.cam_vis = {'L':False,'R':False}
      self.display_counter = {'loc':0,'wp':0,'map':0}
      self.gui = gui
      self.gui.way_map.buff = []
      self.loc = {'x':0.0,'y':0.0,'theta':0.0}      
      rospy.init_node('operator')        
      #Publishers
      gui.drive = rospy.Publisher('/Drive', Float64)
      #Subscriber
      rospy.Subscriber('/LOC/Pose',PoseWithCovarianceStamped, self.location)
      rospy.Subscriber('/LPP/WP',Point, self.get_wp)      
      rospy.Subscriber('/SENSORS/CAM/L', Image, self.imageR_callback)
      rospy.Subscriber('/SENSORS/CAM/R', Image, self.imageL_callback)
      rospy.Subscriber('/PER/Map', Map, self.get_map)
        
    def get_map(self,data):
      if self.display_counter['map'] == 0:
	#if not self.gui.way_map.reading: 
	  minimum = 100
	  maximum = -100
	  buf = []
	  for i in range(data.info.width):
	    for j in range(data.info.height):
	      if data.data[i * data.info.width + j].height != -100:
		if data.data[i * data.info.width + j].height < minimum:
		  minimum = data.data[i * data.info.width + j].height
		if data.data[i * data.info.width + j].height > maximum:
		  maximum = data.data[i * data.info.width + j].height
	      c = chr(int(data.data[i * data.info.width + j].height + 100))
	      buf.append(c)
	      buf.append(c)
	      buf.append(c)
	  buff = string.join(buf, '')
	      
	  ##TODO change set data in map
	  #self.gui.way_map.set_buffer(buf)
	  self.gui.way_map.buff = buff
	#else: print 'reading buffer'
      elif self.display_counter['map'] == COUNT_TIMES:
	self.display_counter['map'] = -1
      self.display_counter['map'] += 1
      #elif self.display_counter['map'] == COUNT_TIMES: self.display_counter['map'] = -1
      #self.display_counter['map'] += 1
      #print minimum,maximum
      
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
      if self.cam_vis['R']:
	self.img = numpy.array(Img.frombuffer("RGB", [PX, PY], data.data, 'raw', "RGB", 0, 1))#.crop((0, PY,PX, PY))#.resize((40, 25)).convert('1')
	pix = gtk.gdk.pixbuf_new_from_array(self.img,gtk.gdk.COLORSPACE_RGB,8)
	pix = pix.scale_simple(100,100,gtk.gdk.INTERP_BILINEAR)
	self.gui.imageR.set_from_pixbuf(pix)

    def imageL_callback(self, data):
      if self.cam_vis['L']:
	self.img = numpy.array(Img.frombuffer("RGB", [PX, PY], data.data, 'raw', "RGB", 0, 1))#.crop((0, PY,PX, PY))#.resize((40, 25)).convert('1')
	pix = gtk.gdk.pixbuf_new_from_array(self.img,gtk.gdk.COLORSPACE_RGB,8)
	pix = pix.scale_simple(100,100,gtk.gdk.INTERP_BILINEAR)
	self.gui.imageL.set_from_pixbuf(pix)