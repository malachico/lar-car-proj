#!/usr/bin/env python
import thread,time
from PIL import ImageTk
import pygtk
import gtk,webkit
import menu

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
  def __init__(self,SIZE,key_down_func):
    # Main window
    self.win = gtk.Window(gtk.WINDOW_TOPLEVEL)
    self.set_main_window(SIZE)
    self.win.connect('key_press_event', key_down_func)    
    
    # Image stuff
    self.imgbox = gtk.HBox() #image box
    self.mainbox.pack_start(self.imgbox,False)
    self.imageR = gtk.Image()#define image
    self.imageL = gtk.Image()#define image
    self.imgbox.pack_start(self.imageR)
    self.imgbox.pack_start(self.imageL)
    self.imgbox.set_size_request(800,300)
    
    # Web stuff #
    # Address and button
    self.address_box =gtk_object(gtk.HBox())
    self.addressbar = gtk.Entry()
    self.web = webkit.WebView()
    self.gobutton = gtk.Button('GO')
    self.gobutton.connect('clicked',self.go_button)
    self.address_box.obj.pack_start(self.addressbar)
    self.address_box.obj.pack_start(self.gobutton,False)
    self.mainbox.pack_start(self.address_box.obj, False)
    self.menu.add_item(("/Web/Show|Hide address bar","<control>H", self.show_hide,0,"<CheckItem>"))
    self.menu.add_item(("/Web/Show|Hide website","<control>H", self.show_hide,1,"<CheckItem>"))
    
    # Web interface
    self.scroller = gtk_object(gtk.ScrolledWindow(),True)
    self.mainbox.pack_start(self.scroller.obj)
    self.scroller.obj.add(self.web)
    
  def init_system(self):
    #start menu 
    self.menu.generate_menu(self.mainbox,self.win)
    self.web.open('http://www.google.com')#'file:///home/lar5/Desktop/ros_ws/src/lar_car/src/map.html')#
    self.mainbox.show()
    self.win.show_all()
    self.address_box.set_show_hide()
    self.scroller.set_show_hide()
  
  def on_key_press_event(self, widget, event):
    key = gtk.gdk.keyval_name(event.keyval)
    print "Key %s (%d) was pressed" % (key, event.keyval)

  
  def show_hide(self,widget,data=None):
    if widget == 0:
      self.address_box.is_shown_toggle()
    elif widget == 1:
      self.scroller.is_shown_toggle()
    else:
      print 'Unknown request ',widget
    
  def set_main_window(self,SIZE):
    self.win.set_position(gtk.WIN_POS_CENTER)
    self.win.set_size_request(SIZE[0],SIZE[1])
    self.win.connect('destroy',self.run_quit)
    # add general box
    self.mainbox = gtk.VBox()
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
    gtk.main_quit()
  #def choose_image(widget):
	#[fn,response] = file_chooser.file_chooser()
	#if response == gtk.RESPONSE_OK:
		#pix = gtk.gdk.pixbuf_new_from_file(fn)
		#image.set_from_pixbuf(pix)  
    
#imgchoosebutton = gtk.Button('Open')
#imgbox.pack_start(imgchoosebutton,False)
#imgchoosebutton.connect('clicked',choose_image)   