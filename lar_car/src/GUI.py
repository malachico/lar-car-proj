#!/usr/bin/env python
import thread,time
from PIL import ImageTk
import pygtk
import gtk,webkit
import menu
from std_msgs.msg import Float64

def change_bg_color(widget,color_string):
  color = widget.get_colormap().alloc_color(color_string)
  style = widget.get_style().copy()
  style.bg[gtk.STATE_NORMAL] = color
  widget.set_style(style)

class gtk_object:
  def __init__(self,obj,init_condition=False):
    self.obj = obj
    self.is_show = init_condition
  def set_show_hide(self):
    if self.is_show:
      self.obj.show()
    else:
      self.obj.hide()
  
  def is_shown_toggle(self):
    if self.is_show:
      self.is_show = False
      self.obj.hide()
      return True
    else:
      self.is_show = True
      self.obj.show()
      return False
      
class car_gui:
  def hello_button(self,widget):
    print "hello world"
  
  def start_driving(self,widget):
    if widget.get_label() == 'Connect':
      widget.set_label('Connected')
      self.drive.publish(Float64(1))
      widget.modify_bg(gtk.STATE_NORMAL, gtk.gdk.Color('green'))
      #change_bg_color(widget,'green')
    else:
      widget.set_label('Connect')
      self.drive.publish(Float64(0))
      widget.modify_bg(gtk.STATE_NORMAL, gtk.gdk.Color('red'))
    
  def __init__(self,SIZE,key_down_func):
    # Main window
    self.win = gtk.Window(gtk.WINDOW_TOPLEVEL)
    self.set_main_window(SIZE)
    self.win.connect('key_press_event', key_down_func)    
    # LEFT BOX
    self.left_box = gtk.VBox()
    self.mainbox.pack_start(self.left_box,False)
    self.left_box.set_size_request(100,800)
    
    # Add Location Frame
    self.location_frame = gtk.Frame(label='Location')
    self.location_frame.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
    self.left_box.pack_start(self.location_frame)
    self.location_frame.modify_bg(gtk.STATE_NORMAL, gtk.gdk.Color('black'))
    self.location_frame.set_size_request(100,0)
    #print dir(self.frame)
    self.loc_txt = gtk.Label()
    #self.loc_txt.show()
    self.location_frame.add(self.loc_txt)

    # Add Way Point Frame
    self.wp_frame = gtk.Frame(label='Way Point')
    self.wp_frame.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
    self.left_box.pack_start(self.wp_frame)
    self.wp_frame.modify_bg(gtk.STATE_NORMAL, gtk.gdk.Color('black'))
    self.wp_frame.set_size_request(100,0)
    #print dir(self.frame)
    self.wp_txt = gtk.Label('x: 0\ny: 0')
    #self.loc_txt.show()
    self.wp_frame.add(self.wp_txt)
    
    # Add Auto/Manual Frame
    self.AM_frame = gtk.Frame(label='Auto/Manual')
    self.AM_frame.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
    self.left_box.pack_start(self.AM_frame)
    self.AM_frame.set_size_request(10,10)
    #Add button
    self.button2 = gtk.Button('Auto')
    self.button2.connect('clicked',self.hello_button)
    self.button2.set_size_request(100,10)
    self.AM_frame.add(self.button2)
    
    # Add drive Frame
    self.drive_frame = gtk.Frame(label='Connect')
    self.drive_frame.set_shadow_type(gtk.SHADOW_ETCHED_OUT)
    self.left_box.pack_start(self.drive_frame)
    #Add connect button
    self.connect_button = gtk.Button('Connect')
    self.connect_button.modify_bg(gtk.STATE_NORMAL, gtk.gdk.Color('red'))
    self.connect_button.connect('clicked',self.start_driving)
    self.drive_frame.add(self.connect_button)
    
    
    # RIGHT BOX
    self.right_box = gtk.VBox()
    self.mainbox.pack_start(self.right_box)
    # Image stuff
    self.imgbox = gtk.HBox() #image box
    self.right_box.pack_start(self.imgbox,False)
    self.imageR = gtk.Image()#define image
    self.imageL = gtk.Image()#define image
    self.imgbox.pack_start(self.imageR)
    self.imgbox.pack_start(self.imageL)
    self.imgbox.set_size_request(800,300)
    
    # Web stuff #
    self.web_box = gtk_object(gtk.VBox(),True)
    self.right_box.pack_start(self.web_box.obj)
    # Address and button
    
    self.address_box =gtk_object(gtk.HBox())
    self.addressbar = gtk.Entry()
    self.web = webkit.WebView()
    self.gobutton = gtk.Button('GO')
    self.gobutton.connect('clicked',self.go_button)
    self.address_box.obj.pack_start(self.addressbar)
    self.address_box.obj.pack_start(self.gobutton,False)
    self.web_box.obj.pack_start(self.address_box.obj,False)
    self.menu.add_item(("/Web/Show|Hide address bar","<control>H", self.show_hide,0,"<CheckItem>"))
    self.menu.add_item(("/Web/Show|Hide website","<control>H", self.show_hide,1,"<CheckItem>"))
    
    # Web interface
    self.scroller = gtk.ScrolledWindow()
    self.web_box.obj.pack_start(self.scroller)
    self.scroller.add(self.web)
    
    
  def init_system(self):
    #start menu 
    self.menu.generate_menu(self.mainbox,self.win)
    self.web.open('http://www.google.com')#'file:///home/lar5/Desktop/ros_ws/src/lar_car/src/map.html')#
    self.mainbox.show()
    self.win.show_all()
    self.address_box.set_show_hide()
    self.web_box.set_show_hide()
  
  def on_key_press_event(self, widget, event):
    key = gtk.gdk.keyval_name(event.keyval)
    print "Key %s (%d) was pressed" % (key, event.keyval)

  
  def show_hide(self,widget,data=None):
    if widget == 0:
      self.address_box.is_shown_toggle()
    elif widget == 1:
      self.web_box.is_shown_toggle()
    else:
      print 'Unknown request ',widget
    
  def set_main_window(self,SIZE):
    self.win.set_position(gtk.WIN_POS_CENTER)
    self.win.set_size_request(SIZE[0],SIZE[1])
    self.win.connect('destroy',self.run_quit)
    # add general box
    self.mainbox = gtk.HBox()
    self.win.add(self.mainbox)
    self.menu = menu.myMenu()
    
  def go_button(self,widget):
    add = self.addressbar.get_text()
    if not (add.startswith('http://') or add.startswith('file://')):
	    add = 'http://' + add
	    self.addressbar.set_text(add)
    self.web.open(add)
  
  def run_main(self):
    self.init_system()
    gtk.main()
    
  def run_quit(self,widget):
    self.drive.publish(Float64(0))
    gtk.main_quit()
  #def choose_image(widget):
	#[fn,response] = file_chooser.file_chooser()
	#if response == gtk.RESPONSE_OK:
		#pix = gtk.gdk.pixbuf_new_from_file(fn)
		#image.set_from_pixbuf(pix)  
    
#imgchoosebutton = gtk.Button('Open')
#imgbox.pack_start(imgchoosebutton,False)
#imgchoosebutton.connect('clicked',choose_image)   