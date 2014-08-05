#!/usr/bin/env python
import rospy
from sensor_msgs.msg import Image
from PIL import Image as Img
from ImageProc import find_position
from geometry_msgs.msg import PoseWithCovarianceStamped,Point
from tf.transformations import euler_from_quaternion
from PIL import ImageFilter
import numpy, math
import sys
PX = 1288
PY =  964
WIDTH = PY
LENGTH = PX
loc = {'x':0,'y':0,'theta':0}
image = 0

def location_callback(obj):
  global loc
  loc['x'] = obj.pose.pose.position.x
  loc['y'] = obj.pose.pose.position.y
  theta = euler_from_quaternion( numpy.array((obj.pose.pose.orientation.x, obj.pose.pose.orientation.y, obj.pose.pose.orientation.z, obj.pose.pose.orientation.w), dtype=numpy.float64))
  loc['theta'] = theta[2]
  
def image_callback(data):
  global image
  image = Img.frombuffer("RGB", [PX, PY], data.data, 'raw', "RGB", 0, 1).crop((0, PY/2,PX, PY))#.resize((40, 25)).convert('1')
  run_path_planner()

def pix2xy(x,y):
    Py = 40
    Px = 25
    FOV = 1.3963  # Field Of (camera's) View
    H = 1.97  # [meters] - location of camera on Z axis
    ###Xpix = 1624  # amount of pixels on x
    ###Ypix = 1224  # amount of pixels on y
    
    alpha = (math.pi-FOV)/2.0
    mini = H*math.tan(alpha)
    beta = (y - Py/2.0)/(Py/2.0)*FOV/2.0
    if x==0:
        xval = 100
    else:
        xval = (Px - x)*mini/x
    yval = xval*math.tan(beta)
    #sys.stdout.write( 'x:%d  y:%d  xval:%.2f  yval:%.2f'%(x,y,xval,yval))
    return [xval-0.351,yval-0.0341]

def getWP(a,b,loc):
    WP = pix2xy(b,a)
    WPx = WP[0]*math.cos(loc['theta']) + WP[1]*math.sin(loc['theta'])
    WPy = -WP[0]*math.sin(loc['theta']) + WP[1]*math.cos(loc['theta'])
    WP[0] = WPx + loc['x']
    WP[1] = -WPy + loc['y']
    #sys.stdout.write( '  WPx:%.2f  WPy:%.2f\n'%(WP[0],WP[1]))
    return WP
  
def run_path_planner():
  im = image.resize((40,25)).convert('1').filter(ImageFilter.MedianFilter(3)).filter(ImageFilter.MedianFilter(5)).point(lambda p: p> 0 and 255)
  im.show()
  pixels = list(im.getdata())
  autopoint = find_position(pixels)
  WP = getWP(autopoint[0],autopoint[1],loc)
  msg = Point()
  msg.x = WP[0]
  msg.y = WP[1]
  wp_publisher.publish(msg)
    

if __name__ == "__main__":
  print 'Starting Local path planner...'
  rospy.init_node('path_planner')
  rospy.Subscriber('/SENSORS/CAM/R', Image, image_callback)
  rospy.Subscriber('/LOC/Pose',PoseWithCovarianceStamped, location_callback)
  wp_publisher = rospy.Publisher('/LPP/WP', Point)
  print 'Node is Running. Subscribe to /LPP/WP for result'
  rospy.spin()
  print 'LPP: "Goodbye..."'
  