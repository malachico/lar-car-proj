import simpleguitk as simplegui
import numpy
from math import isnan
from PIL import Image as Img
from PIL import ImageTk
import ImageFilter
from aux_func import *
from ekf_data_collector import ekfDataLog
from ImageProc import find_position, colorize
import cv2,time

WIDTH = 400
LENGTH = 250
#WPlist = [[99.0,0.0],[150.0,50.0],[150.0,100.0]]
index = 1
auto = 0
auto_flag = 1
show_txt = 0
WP = [0,0]
gas = 0
point = [-1,-1]
autopoint = [-1,-1]
pos_var_log_x = []
pos_var_log_y = []
real_x = []
real_y = []
loger = ekfDataLog()

def location_handler():
    global controller, WP, gas, autopoint, auto, pos_var_log_x, pos_var_log_y
    im = controller.img._image.resize((40, 25)).convert('1').filter(ImageFilter.MedianFilter(3)).filter(ImageFilter.MedianFilter(5)).point(lambda p: p> 0 and 255)
    pixels = list(im.getdata())
    autopoint = find_position(pixels)
    autopoint[0]*=10
    autopoint[1]*=10
    if auto:
        WP = getWP(autopoint[0]- WIDTH/2,autopoint[1],controller.loc)
    d,theta = calc_error(controller.loc,WP)
    g,s = PID(d,theta)
    controller.speed.set(g*gas*3)
    if isnan(controller.speed.get()):
       print "Warning: controller speed is NAN. Reset to 0."
       controller.speed.set(0)
    controller.steer.set(s)
    controller.publisher()
    pos_var_log_x.append(controller.loc['x'])
    pos_var_log_y.append(controller.loc['y'])
    real_x.append(controller.real_loc['x'])
    real_y.append(controller.real_loc['y'])    

def mouse_handler(position):
    global WP, controller, index, point, alldata
    point = position
    arr = list(controller.img._image.resize((WIDTH,LENGTH)).getdata())
    print arr[position[1]*WIDTH+position[0]]
    #to_file(position[0],position[1],controller,alldata)
    a = position[0] - WIDTH/2
    b = position[1]# - LENGTH/2
    WP = getWP(a,b,controller.loc)

def keydown_handler(key):
    global gas,auto,auto_flag,label
    if key == simplegui.KEY_MAP['up']:
        gas = 1
    if key == simplegui.KEY_MAP['space'] and auto_flag:
        auto = abs(auto-1)
        auto_flag = 0
        if not auto:
            label.set_text('Manual')
        else:
            label.set_text('Auto')


def keyup_handler(key):
    global gas, auto_flag
    if (key == simplegui.KEY_MAP['up']):
        gas = 0
    if key == simplegui.KEY_MAP['space']:
        auto_flag = 1
old_im = 0
def draw(canvas):
    global controller, WP, point, autopoint, show_txt, old_im
    img = controller.img
    width = img.get_width()
    height = img.get_height()
    canvas.draw_image(img, [width/2.0, height/2.0],  [width, height], [WIDTH/2, LENGTH/2], [WIDTH, LENGTH])#controller.draw(canvas, WIDTH, LENGTH)#, width=800, height=800)
    if show_txt:
        import math, operator
        #print img._image.getdata(),height,width
        h1 = img._image.histogram()
        h2 = old_im._image.histogram()
        rms = math.sqrt(reduce(operator.add,map(lambda a,b: (a-b)**2, h1, h2))/len(h1))
        print rms


    else:
        pass
        # print dir(img._image)
        # img._image.show()
        # rospy.sleep(1)
        # show_txt = True
        # img = controller.start_img
        # img._image.show()
        # rospy.sleep(5)
        # #canvas.draw_image(img, [width/2.0, height/2.0],  [width, height], [WIDTH/2, LENGTH/2], [WIDTH, LENGTH])#controller.draw(canvas, WIDTH, LENGTH)#, width=800, height=800)
        # canvas.draw_image(img, [width/2.0, height/2.0],  [width, height], [WIDTH/2, LENGTH/2], [WIDTH, LENGTH])
    old_im = img
    try:
        a = 0
    except:
    	print "no image to display"
    #draw text
    if show_txt:
        locstr = 'x     : %.2f\ny     : %.2f\nyaw: %.2f'%(controller.loc['x'],controller.loc['y'],controller.loc['theta']*180/math.pi)
        canvas.draw_text(locstr, (310,80),12,'Blue')
        d,theta = calc_error(controller.loc,WP)
        erstr = 'Dist: %.2f\nOri  : %.2f'%(d,theta)
        canvas.draw_text(erstr, (10,60),12,'Red')
    #draw circle
    if point[0]!=-1:
    	pass
        #canvas.draw_circle(point, 20, 4, 'Green')
    if autopoint[0]!=-1:
    	pass
        #canvas.draw_circle(autopoint, 20, 4, 'Aqua')

def save():
    global loger
    '''global alldata
    save_data(alldata)
    print "generating Matlab files in matlab/data directory"
    save_data_to_mat('dataset')
    print "files generated"'''
    print "---------------------"
    #save_matlab(pos_var_log_x,"x")
    #save_matlab(pos_var_log_y,"y")
    #save_matlab(real_x,"r_x")
    #save_matlab(real_y,"r_y")
    x,y,the,lat, longi, acc, gyro, ori = loger.extractData()
    save_matlab(x,"x","data/")
    save_matlab(y,"y","data/")
    save_matlab(the,"yaw","data/")
    save_matlab(lat,"lat","data/")
    save_matlab(longi,"long","data/")
    save_matlab(acc,"acc","data/")
    save_matlab(gyro,"gyro","data/")
    save_matlab(ori,"orient","data/")

def show_text():
    global show_txt
    show_txt = abs(show_txt-1)


controller = ROS()
frame = simplegui.create_frame("CAR_GUI", WIDTH, LENGTH)
label = frame.add_label('Manual')
frame.add_button('text',show_text)
frame.set_draw_handler(draw)
frame.set_mouseclick_handler(mouse_handler)
frame.set_keydown_handler(keydown_handler)
frame.set_keyup_handler(keyup_handler)
frame.add_button('save data',save)
loc_timer = simplegui.create_timer(100, location_handler)
loc_timer.start()
frame.start()


