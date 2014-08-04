import simpleguitk as simplegui
import serial
import thread
import time
from stringBufferHandler import myBuffer

#### Global Variables #####
try:
  ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=0)
except:
  ser = 0
  print '/dev/ttyUSB0 is not connected'
W = 400
H = 400
curGreen = (3*W/4,3*H/4)
curRed = (3*W/4,3*H/4)
buff = myBuffer(40)
#### Additional functions #####
def pos2val(pos):
    return int(((H - pos[1]) - 100) * 9.99/2.5)

def val2MotorString(val):
    if (val) > 900:
      val = 900
    if val < -900:
      val = -900
    st = ''
    if val >= 0:
        st = '+'
    n = str(val)
    
    return st + n.zfill(3)
        
#### GUI functions ####
def mouse_handler(position):
    global curGreen,curRed
    if not (position[1] < H / 2):
        if position[0] > 20 and position[0] < 70:
            curGreen = position
        elif position[0] > 120 and position[0] < 170:
            curRed = position
        write_port()

def key_down(key):
    global curGreen, curRed
    if key == simplegui.KEY_MAP['a']:
        curGreen = (3*W/4,3*H/4)
    if key == simplegui.KEY_MAP['s']:
        curRed = (3*W/4,3*H/4)
    write_port()

def draw(canvas):
    global buff
    canvas.draw_polygon([(20, H), (20, curGreen[1]), (70, curGreen[1]), (70, H)], 12, 'Green')
    canvas.draw_polygon([(120, H), (120, curRed[1]), (170, curRed[1]), (170, H)], 12, 'Red')
    canvas.draw_text(pos2val(curGreen), (30, H / 2 - 20), 12, 'Green')
    canvas.draw_text(pos2val(curRed), (130, H / 2 - 20), 12, 'Red')
    canvas.draw_text(buff._string, (20,H/2-40),12,'Blue')

#### Serial function ####
def read_port(threadName, delay):
    global ser,buff
    while ser.isOpen():
        line =ser.readline() # should block, not take anything less than 14 bytes
        if line:
            buff.add_to_buffer(line+'\n')
            
        time.sleep(delay)

def write_port():
      global ser,curRed,curGreen,buff
      mot1 = val2MotorString(pos2val(curGreen))      
      mot2 = val2MotorString(pos2val(curRed))
      st = 'M'+mot1+','+mot2+'000'
      buff.add_to_buffer('sending '+st+'\n')
      ser.write(st)
      time.sleep(0.1)

def open_port():
    global ser,buff
    ser.open()
    print ("connected to: " + ser.portstr)
    buff.add_to_buffer("connected to: " + ser.portstr+'\n')
    thread.start_new_thread( read_port, ("Thread-1", 0.01, ) )

def close_port():
    global ser,buff
    print ("closing port " + ser.portstr)
    buff.add_to_buffer("Closing port " + ser.portstr+'\n')
    ser.close()


#### GUI frame init ####
frame = simplegui.create_frame('driver_gui',W,H)
frame.set_canvas_background('White')
frame.add_button('OpenPort',open_port)
frame.add_button('ClosePort',close_port)
frame.add_button('Write2Port',write_port)
frame.set_draw_handler(draw)
frame.set_mouseclick_handler(mouse_handler)
frame.set_keydown_handler(key_down)
frame.start()
