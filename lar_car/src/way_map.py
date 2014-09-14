#!/usr/bin/env python

# example drawingarea.py

import pygtk
pygtk.require('2.0')
import gtk
import operator
import time
import string
import thread
  
class MapViewer:
    def onclick (self,box, event):
	self.reading = not self.reading
	print self.reading
     
    def __init__(self,window,SIZE):
        self.reading = True
        self.size = SIZE
        self.MapBox = gtk.EventBox()
        
	self.MapBox.connect ('button-press-event', self.onclick)
        self.area = gtk.DrawingArea()
        self.area.set_size_request(SIZE,SIZE)
        self.MapBox.add(self.area)
        window.pack_start(self.MapBox)
        self.area.set_events(gtk.gdk.POINTER_MOTION_MASK |
                             gtk.gdk.POINTER_MOTION_HINT_MASK )
        
        #self.area.connect("expose-event", self.area_expose_cb)
        thread.start_new_thread(self.set_buffer,(0,))
        self.area.show()
        
        self.MapBox.show()      
      
    def area_expose_cb(self, area, event):
        print area, event
        self.set_buffer(0)
        
    def draw_rgb_image(self, x, y,buff):
        
        gc = self.area.get_style().fg_gc[gtk.STATE_NORMAL]
        self.area.window.draw_rgb_image(gc, x, y, self.size, self.size,gtk.gdk.RGB_DITHER_NONE, buff, self.size*3)
        
    def set_buffer(self,buf):
      b = self.size*3*self.size*['\0']
      for i in range(self.size):
	  for j in range(self.size):
	      b[3*self.size*i+3*j] = chr(abs(255-i)%255)
	      b[3*self.size*i+3*j+1] = chr(abs(255-abs(i-j))%255)
	      b[3*self.size*i+3*j+2] = chr(abs(255-j)%255)
      temp_buf = string.join(b, '')
      time.sleep(3)
      print 'set_buffer'
      while  True:
	if self.reading:
	  buff = temp_buf
	else:
	  buff = self.buff
	  #b = self.size*3*self.size*['\0']
	  #for i in range(self.size):
	      #for j in range(self.size):
		  #b[3*self.size*i+3*j] = chr(self.buff[i*self.size + j])
		  #b[3*self.size*i+3*j+1] = chr(self.buff[i*self.size + j])
		  #b[3*self.size*i+3*j+2] = chr(self.buff[i*self.size + j])
	
	self.draw_rgb_image(0,0,buff)
	time.sleep(1)
	print 'here'

#def main():
    
    #gtk.main()
    #return 0

#if __name__ == "__main__":
    #window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    #window.set_title("Drawing Area Example")
    #window.connect("destroy", lambda w: gtk.main_quit())
    #SIZE = 250
    #box = gtk.HBox()
    #window.add(box)
    #box.show()
    #mapv = MapViewer(box,SIZE)
    #window.show()
    #main()
