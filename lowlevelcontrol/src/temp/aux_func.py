import rospy
from std_msgs.msg import Float64
from std_msgs.msg import Int8
from geometry_msgs.msg import PoseWithCovarianceStamped
from gazebo_msgs.msg import ModelStates
from sensor_msgs.msg import Image
import simpleguitk as simplegui
from PIL import Image as Img
from PIL import ImageFilter
from tf.transformations import euler_from_quaternion
import numpy, math
import scipy.io


PX = 1624
PY =  1224

class var:
    def __init__(self):
        self.var = 0
    def set(self, val):
        self.var = val
    def get(self):
        return self.var
    def toggle(self):
        if self.var == 0:
            self.var = 1
        else:
            self.var = 0

class subscriber:
    def __init__(self,topic,type_num):
        if type_num == 1:
            type = Float64
        if type_num == 2:
            type = Image
        if type_num == 3:
            type = ModelStates
        self._sub = rospy.Subscriber(topic,type, self.set_data)
        self.data = 0

    def set_data(self,data):
        self.data = data

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

def sign(number):
    if number==0:
        return 0
    return 1

def to_file(a,b,controller,alldata):
        img = controller.img._image#.resize((W, H)).convert('1').point(lambda p: p> 90 and 255).filter(ImageFilter.MedianFilter(3)).filter(ImageFilter.MedianFilter(5))
        pixels = list(img.getdata())
        width, height = img.size
        pixels = [pixels[i * width:(i + 1) * width] for i in xrange(height)]
        dat = []
        for val in pixels:
            for v in val:
                    dat.append(v)
        a= a/10
        b = b/10
        alldata.appendLinked(dat, a+(b)*40)#,numtoind(controller.speed.get(),numofvec = 10)

def save_data(alldata):
        print "Saving data set..."
        alldata.saveToFile('dataset')
        print "Data set saved!"
        print "--------------------"

def point_selection(x,y):
    FOV = 1.3963  # Field Of (camera's) View
    H = 1.97  # [meters] - location of camera on Z axis
    ###Xpix = 1624  # amount of pixels on x
    ###Ypix = 1224  # amount of pixels on y
    WIDTH = 400
    LENGTH = 250
    alpha = (math.pi-FOV)/2.0
    mini = H*math.tan(alpha)
    beta = y/(LENGTH/2.0)*FOV/2.0
    if x==0:
        xval = 100
    else:
        xval = WIDTH/2*mini/x
    yval = xval*math.tan(beta)
    print "x: ", x, "y: ", y,"xval: ",xval," ybal: ",yval
    return [xval-0.351,yval-0.0341]

def getWP(a,b,loc):
    WP = point_selection(b,a)
    WPx = WP[0]*math.cos(loc['theta']) + WP[1]*math.sin(loc['theta'])
    WPy = -WP[0]*math.sin(loc['theta']) + WP[1]*math.cos(loc['theta'])
    WP[0] = WPx + loc['x']
    WP[1] = -WPy + loc['y']
    return WP

def PID(dist,dt):
    s = dt/45
    g = dist*0.01
    if g<0.05:
        g = 0.05
    if dist<1.0:
        g = 0
    return g,s

class ROS:
    def __init__(self):
        self.speed = var()
        self.steer = var()
        self.steerlog = []
        self.brake = var()
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
        self._image = rospy.Subscriber('/SENSORS/CAM/TEST', Image, self.image_callback)
        self._steerHistory = rospy.Subscriber('/drivingVic/hand_wheel/state', Float64, self.log_steer)
        self.img = simplegui.load_image('start.jpg')
        self.start_img = simplegui.load_image('start.jpg')

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
        '''i = 0
        for objecti in obj.name:
            if objecti == 'drivingVic':
                val = i
                break
            i += 1
        theta = euler_from_quaternion( numpy.array((obj.pose[val].orientation.x, obj.pose[val].orientation.y, obj.pose[val].orientation.z, obj.pose[val].orientation.w), dtype=numpy.float64))
        self.loc['x'] = int(obj.pose[val].position.x*100)/100.0
        self.loc['y'] = int(obj.pose[val].position.y*100)/100.0
        self.loc['theta'] = int(theta[2]*100)/100.0
        '''
        self.loc['x'] = obj.pose.pose.position.x
        self.loc['y'] = obj.pose.pose.position.y
        theta = euler_from_quaternion( numpy.array((obj.pose.pose.orientation.x, obj.pose.pose.orientation.y, obj.pose.pose.orientation.z, obj.pose.pose.orientation.w), dtype=numpy.float64))
        self.loc['theta'] = theta[2]

    def image_callback(self, data):
        self.img._image = Img.frombuffer("RGB", [PX, PY], data.data, 'raw', "RGB", 0, 1).crop((0, PY/2,PX, PY))#.resize((40, 25)).convert('1')

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
    def draw(self, canvas, WIDTH, LENGTH, width=0, height=0,offset=0):
        if not width:
            img = self.img
            width = img.get_width()
            height = img.get_height()
        else:
            img = self.img
            img._image=img._image.resize((width, height))
	    #img._image = img._image.convert('L')
	    #print img._image.histogram()
        canvas.draw_image(img, [width/2.0, height/2.0],  [width, height], [WIDTH/2, LENGTH/2+offset], [WIDTH, LENGTH])
def numtoind(num, numofvec = 30.0, min = -1.0, max = 1.0):
        if num>max:
            num = max
        if num<min:
            num = min
        a = (max-min)/numofvec
        ind = math.floor((num-min)/a)
        if ind>=numofvec:
            ind = numofvec-1
        return int(ind)
def numtovec(num, numofvec =30.0, min = -1.0, max = 1.0):
        a = (max-min)/numofvec
        ind = math.floor((num-min)/a)
        arr = numpy.zeros(numofvec)
        if ind>=numofvec:
            ind = numofvec-1
        arr[ind] = 1
        return arr        
def vectonum(vec, numofvec = 30, min = -1.0, max = 1.0):
        vec = vectorizeme(vec, numofvec)
        for ind in range(numofvec):
            if vec[ind] == 1:
                break
        a = (max-min)/numofvec
        b=min+float(ind+1)*a
        return b
def vectorizeme(vec, numofvec):
        min = 0
        max = 0
        for i in range(numofvec):
            if vec[i] > vec[max]:
                maxi = max
                max = i
        arr = numpy.zeros(numofvec)
        arr[max] = 1
        return arr
def save_data_to_mat(data_name, dir='matlab/data'):
        print "opening file: ", data_name
        alldata = ClassificationDataSet.loadFromFile(data_name)
        x = alldata['input']
        str = dir+'/'+"x.mat"
        print "saving input to file: "+str
        scipy.io.savemat(str, mdict={'x': x})
        y = alldata['target']
        str = dir+'/'+"y.mat"
        print "saving target to file: y.mat"
        scipy.io.savemat(str, mdict={'y': y})
        print "done..."
#save_data_to_mat('dataset')
def save_matlab(data,name,dir=None):
        print "saving ", name
        str = dir + name + ".mat"
        scipy.io.savemat(str, mdict={name: data})
