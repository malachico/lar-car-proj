#!/usr/bin/env python 
import pygame
import thread
import time

class joystick_control():
    def __init__(self):
        pygame.init()
        pygame.joystick.init()
        joystick_count = pygame.joystick.get_count()
        if joystick_count<1:
            print "Joystick is not connected"
            return
        self.joystick = pygame.joystick.Joystick(0)
        self.joystick.init()
        self._numofaxes = self.joystick.get_numaxes()
        self._numofbuttons = self.joystick.get_numbuttons()
        self._hats = self.joystick.get_numhats()
        
        self.axes = [0]*self._numofaxes
        self.buttons = [0]*self._numofbuttons
        self.hats = [0]*self._hats
        
        thread.start_new_thread(self.update_handler,(10,))
        
    def get_axes(self):
        for i in range(self._numofaxes):
            self.axes[i] = self.joystick.get_axis( i )
        return self.axes
    def get_buttons(self):
        for i in range(self._numofbuttons):
            self.buttons[i] = self.joystick.get_button( i )
        return self.buttons    
    def update_handler(self,idle):
        while True:
	  pygame.event.get()
	  self.get_buttons() 
	  self.get_axes()
	  time.sleep(10 / 1000.0)
        
    def get_all(self):
        return [self.buttons, self.axes]
    
    def test_joystick(self):
        print "number of joysticks: ", pygame.joystick.get_count()
        print "number of axes: ", self._numofaxes
        print "number of buttons: ", self._numofbuttons
        for i in range(10):
            but, ax = self.get_all()
            print "buttons: ", but           
            print "axes: ", ax           
            time.sleep(1)

if __name__ == "__main__":
  print 'searching for joysticks'
  joy = joystick_control()
  joy.test_joystick()

