#!/usr/bin/env python
import thread,time
from PIL import ImageTk
import pygtk
import gtk,webkit
import menu
import os
from std_msgs.msg import Float64

   
class car_gui:
   
  def __init__(self,SIZE):
    ##### Main window ###
    ##### ###############
    self.win = gtk.Window(gtk.WINDOW_TOPLEVEL)
    self.win.set_position(gtk.WIN_POS_CENTER)
    self.win.set_size_request(SIZE[0],SIZE[1])
    self.win.connect('destroy',self.run_quit)
    #### add general box
    self.mainbox = gtk.HBox()
    self.win.add(self.mainbox)  
    
    ##### RIGHT BOX ###
    ##### #############
    self.right_box = gtk.VBox()
    self.mainbox.pack_start(self.right_box)
    #### Web stuff
    self.web_box = gtk.VBox()
    self.right_box.pack_start(self.web_box,True)
    #### Address and button
    self.address_box =gtk.HBox()
    self.addressbar = gtk.Entry()
    self.gobutton = gtk.Button('GO')
    self.gobutton.connect('clicked',self.go_button)
    self.address_box.pack_start(self.addressbar)
    self.address_box.pack_start(self.gobutton,False)
    self.web_box.pack_start(self.address_box,False)
    #### Web interface
    self.scroller = gtk.ScrolledWindow()
    self.web_box.pack_start(self.scroller)
    self.web = webkit.WebView()
    loc = 'file://'+os.path.dirname(os.path.realpath(__file__))+'/../django/map.html'
    #print loc
    self.web.open(loc)#'http://www.google.com')#
    self.scroller.add(self.web)
  
  def init_system(self):
    self.mainbox.show()
    self.win.show_all()
    
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