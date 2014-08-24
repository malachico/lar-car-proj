#!/usr/bin/env python 
##import pygame
##import time

##class joystick_control():
    ##def __init__(self):
        ##pygame.init()
        ##pygame.joystick.init()
        ##joystick_count = pygame.joystick.get_count()
        ##if joystick_count<1:
            ##print "Joystick is not connected"
            ##return
        ##self.joystick = pygame.joystick.Joystick(0)
        ##self.joystick.init()
        ##self._numofaxes = self.joystick.get_numaxes()
        ##self._numofbuttons = self.joystick.get_numbuttons()
        ##self._hats = self.joystick.get_numhats()
        
        ##self.axes = [0]*self._numofaxes
        ##self.buttons = [0]*self._numofbuttons
        ##self.hats = [0]*self._hats
        

    ##def get_axes(self):
        ##for i in range(self._numofaxes):
            ##self.axes[i] = self.joystick.get_axis( i )
        ##return self.axes
    ##def get_buttons(self):
        ##for i in range(self._numofbuttons):
            ##self.buttons[i] = self.joystick.get_button( i )
        ##return self.buttons    
    ##def update_handler(self):
        ##pygame.event.get()
        ##self.get_buttons() 
        ##self.get_axes()
        
    ##def get_all(self):
        ##return [self.buttons, self.axes]
    ##def test_joystick(self):
        ##print "number of joysticks: ", pygame.joystick.get_count()
        ##print "number of axes: ", self._numofaxes
        ##print "number of buttons: ", self._numofbuttons
        ##for i in range(3):
            ##but, ax = self.get_all()
            ##print "buttons: ", but
            ##print "axes: ", ax
            ##time.sleep(1)

##if __name__ == "__main__":
  ##print 'searching for joysticks'
  ##joy = joystick_control()
  ##joy.test_joystick()

import pygame

# Define some colors
BLACK    = (   0,   0,   0)
WHITE    = ( 255, 255, 255)

# This is a simple class that will help us print to the screen
# It has nothing to do with the joysticks, just outputing the
# information.
class TextPrint:
    def __init__(self):
        self.reset()
        self.font = pygame.font.Font(None, 20)

    def prints(self, screen, textString):
        textBitmap = self.font.render(textString, True, BLACK)
        screen.blit(textBitmap, [self.x, self.y])
        self.y += self.line_height

    def reset(self):
        self.x = 10
        self.y = 10
        self.line_height = 15

    def indent(self):
        self.x += 10

    def unindent(self):
        self.x -= 10


pygame.init()

# Set the width and height of the screen [width,height]
size = [500, 700]
screen = pygame.display.set_mode(size)

pygame.display.set_caption("My Game")

#Loop until the user clicks the close button.
done = False

# Used to manage how fast the screen updates
clock = pygame.time.Clock()

# Initialize the joysticks
pygame.joystick.init()

# Get ready to print
textPrint = TextPrint()

# -------- Main Program Loop -----------
while done==False:
    # EVENT PROCESSING STEP
    for event in pygame.event.get(): # User did something
        if event.type == pygame.QUIT: # If user clicked close
            done=True # Flag that we are done so we exit this loop

        # Possible joystick actions: JOYAXISMOTION JOYBALLMOTION JOYBUTTONDOWN JOYBUTTONUP JOYHATMOTION
        if event.type == pygame.JOYBUTTONDOWN:
            print("Joystick button pressed.")
        if event.type == pygame.JOYBUTTONUP:
            print("Joystick button released.")


    # DRAWING STEP
    # First, clear the screen to white. Don't put other drawing commands
    # above this, or they will be erased with this command.
    screen.fill(WHITE)
    textPrint.reset()

    # Get count of joysticks
    joystick_count = pygame.joystick.get_count()

    textPrint.prints(screen, "Number of joysticks: {}".format(joystick_count) )
    textPrint.indent()

    # For each joystick:
    for i in range(joystick_count):
        joystick = pygame.joystick.Joystick(i)
        joystick.init()

        textPrint.prints(screen, "Joystick {}".format(i) )
        textPrint.indent()

        # Get the name from the OS for the controller/joystick
        name = joystick.get_name()
        textPrint.prints(screen, "Joystick name: {}".format(name) )

        # Usually axis run in pairs, up/down for one, and left/right for
        # the other.
        axes = joystick.get_numaxes()
        textPrint.prints(screen, "Number of axes: {}".format(axes) )
        textPrint.indent()

        for i in range( axes ):
            axis = joystick.get_axis( i )
            textPrint.prints(screen, "Axis {} value: {:>6.3f}".format(i, axis) )
        textPrint.unindent()

        buttons = joystick.get_numbuttons()
        textPrint.prints(screen, "Number of buttons: {}".format(buttons) )
        textPrint.indent()

        for i in range( buttons ):
            button = joystick.get_button( i )
            textPrint.prints(screen, "Button {:>2} value: {}".format(i,button) )
        textPrint.unindent()

        # Hat switch. All or nothing for direction, not like joysticks.
        # Value comes back in an array.
        hats = joystick.get_numhats()
        textPrint.prints(screen, "Number of hats: {}".format(hats) )
        textPrint.indent()

        for i in range( hats ):
            hat = joystick.get_hat( i )
            textPrint.prints(screen, "Hat {} value: {}".format(i, str(hat)) )
        textPrint.unindent()

        textPrint.unindent()


    # ALL CODE TO DRAW SHOULD GO ABOVE THIS COMMENT

    # Go ahead and update the screen with what we've drawn.
    pygame.display.flip()

    # Limit to 20 frames per second
    clock.tick(20)

# Close the window and quit.
# If you forget this line, the program will 'hang'
# on exit if running from IDLE.
pygame.quit ()