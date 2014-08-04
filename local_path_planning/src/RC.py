import simpleguitk as simplegui
import time 
import random
from pybrain.datasets            import ClassificationDataSet
from PIL import Image as Img
from PIL import ImageTk
import ImageFilter
from joystick_controll import joystick_control
from aux_func import var,  ROS, numtovec, numtoind, save_data_to_mat
nb_class = 30
H = 25
W = 40
alldata = ClassificationDataSet(H*W+20, target=2,nb_classes=10)


# initialize globals

TRESHOLD = 95
sim_time = 0
WIDTH = 500
LENGTH = 500
SPEED = 0.0
STEER = 0.0
HB = 1
hbrel = 1
rel = 1
def keydown(key):
    global controller, SPEED, STEER,frame
    if key == simplegui.KEY_MAP['space']:
        STEER= 0
        controller.steer.set(STEER)
    if key == simplegui.KEY_MAP['e']:
        SPEED = 1        
    if key == simplegui.KEY_MAP['d']:
       SPEED = -1
    if key == simplegui.KEY_MAP['m']:
        SPEED = 666            
    if key == simplegui.KEY_MAP['x']:
        SPEED = 0
        controller.speed.set(SPEED)
        controller.brake.set(10)
    if key == simplegui.KEY_MAP['s']:
        STEER = 1
    if key == simplegui.KEY_MAP['f']:
        STEER = -1
    if key == simplegui.KEY_MAP['a']:
        STEER = 10
    if key == simplegui.KEY_MAP['g']:
        STEER = -10        
    if key == simplegui.KEY_MAP['down']:
        controller.hb.toggle()
    if key == simplegui.KEY_MAP['up']:
        controller.direction.set(1) 
    if key == simplegui.KEY_MAP['p']:
            to_file()
    if key == simplegui.KEY_MAP['q']:
        print "goodbye"
def keyup(key):
    global controller, SPEED, STEER
    if key == simplegui.KEY_MAP['e']:
        SPEED = 0
        pass#controller.speed.set(0)   
    if key == simplegui.KEY_MAP['d']:
        SPEED = 0
        pass#controller.brake.set(0)
    if key == simplegui.KEY_MAP['x']:
        controller.brake.set(0)
    if key == simplegui.KEY_MAP['s']:
        controller.img._image.save("image_bw","png")
        pass#controller.steer.set(0)   
    if key == simplegui.KEY_MAP['f']:
        STEER = 0
    if key == simplegui.KEY_MAP['a']:
        STEER = 0
        pass#controller.steer.set(0)   
    if key == simplegui.KEY_MAP['g']:
        STEER = 0        
        pass#controller.steer.set(0)        
    if key == simplegui.KEY_MAP['down']:
        pass
    if key == simplegui.KEY_MAP['up']:
        pass

def to_file():
        global controller, alldata
        img = controller.img._image.resize((W, H)).convert('L').point(lambda p: p> 90 and 255).filter(ImageFilter.MedianFilter(3)).filter(ImageFilter.MedianFilter(5))
        pixels = list(img.getdata())
        width, height = img.size
        pixels = [pixels[i * width:(i + 1) * width] for i in xrange(height)]
        dat = []
        for val in pixels:
            for v in val:
                    dat.append(v)
        [dat.append(val) for val in controller.steerlog]
        alldata.appendLinked(dat, [numtoind(controller.steer.get(), numofvec = 10)])#,numtoind(controller.speed.get(),numofvec = 10)
        #data.append(dat)
        #outputwriter.writerow(data)

def button_handler():
    global output_writer, save_but, alldata
    if save_but.get_text() == 'save':
        print "starting to save. ", len(alldata), " samples in database"
        save_but.set_text('stop')
        output_writer.start()
    else:
        print "finished saving. ", len(alldata), " samples in database"
        save_but.set_text('save')
        output_writer.stop()        
    
def draw(canvas):
        global controller
        controller.draw(canvas, WIDTH, LENGTH)#, width=800, height=800)

def controller_handler():    
        global controller, label
        controller.publisher()
        label.set_text('Gas: ' +str(int(controller.speed.get()*10)/10.0)+'\nSteer: '+str(int(controller.steer.get()*10)/10.0))

def save_data():
        global alldata
        print "Saving data set..."
        alldata.saveToFile('dataset')
        print "Data set saved!"
        print "--------------------"
        print "generating Matlab files in matlab/data directory"
        save_data_to_mat('dataset')
        print "files generated"
        print "---------------------"

def joystick_handler():    
        global sim_time, controller, SPEED, STEER, joystick_controller, HB, hbrel, rel
        inp = joystick_controller.get_all()
        if inp[0][4] and rel:
            rel = 0
            button_handler()
        if inp[0][5] and rel:
            rel = 0
            save_data()
        if not inp[0][5] and not inp[0][4]:
            rel = 1            
        if inp[0][11] and hbrel:
            hbrel = 0
            if HB == 1:
                HB = 0
            else:
                HB = 1
        if inp[0][11]==0:
            hbrel = 1
        gear = inp[0][12]+inp[0][13]*2+inp[0][14]*3+inp[0][15]*4+inp[0][16]*5
        controller.speed.set((inp[1][2]-1)*(-gear)/5.0)
        controller.steer.set(-inp[1][0]*3.14/2.0)
        controller.brake.set((inp[1][3]-1)*(-5))
        controller.hb.set(HB)


# create frame
controller = ROS()
joystick_controller = joystick_control()
#joystick_controller.test_joystick()
frame = simplegui.create_frame("CAR_GUI", WIDTH, LENGTH)
label = frame.add_label('My label')
frame.set_draw_handler(draw)
frame.set_keydown_handler(keydown)
frame.set_keyup_handler(keyup)
output_writer = simplegui.create_timer(500, to_file)
save_but = frame.add_button("save", button_handler)
controller_timer = simplegui.create_timer(10, controller_handler)
joystick_timer = simplegui.create_timer(50, joystick_handler)

# start frame
controller_timer.start()
joystick_timer.start()
frame.start()
