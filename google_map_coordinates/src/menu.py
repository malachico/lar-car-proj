#!/usr/bin/env python

# example menu.py

import pygtk
pygtk.require('2.0')
import gtk
import os
import subprocess

def add_all_menu_items(me):
  #Packages Items
  me.menu.add_item(("/Packages/Localization",         None, run_subprocess,1,None))
  me.menu.add_item(("/Packages/Local path planning",  None, run_subprocess,2,None))
  me.menu.add_item(("/Packages/Low level control",    None, run_subprocess,3,None))
  me.menu.add_item(("/Packages/Stereo Package",    None, run_subprocess,6,None))
  me.menu.add_item(("/Packages/sep1",                 None, None,0,"<Separator>"))
  me.menu.add_item(("/Packages/Run all packages",     "<control>A", run_subprocess,4,None))
  
  
  #Web Items
  me.menu.add_item(("/Web/Show|Hide address bar","<control>H", me.show_hide,0,"<CheckItem>"))
  me.menu.add_item(("/Web/Show|Hide website","<control>H", me.show_hide,1,"<CheckItem>"))
  me.menu.add_item(("/Web/sep2",                 None, None,0,"<Separator>"))
  me.menu.add_item(("/Web/Start Server",    None, run_subprocess,5,None))
  

def run_subprocess(widget,data=None):
  if widget == 1:
    st = os.path.dirname(os.path.realpath(__file__)) + '/../../scripts/localization.sh'
  elif widget == 2:
    st = os.path.dirname(os.path.realpath(__file__)) + '/../../scripts/lpp.sh'
  elif widget == 3:
    st = os.path.dirname(os.path.realpath(__file__)) + '/../../scripts/llc.sh'
  elif widget == 4:
    st = os.path.dirname(os.path.realpath(__file__)) + '/../../scripts/run_all.sh'
  elif widget == 5:
    st = os.path.dirname(os.path.realpath(__file__)) + '/../../scripts/google_map_server.sh'  
  elif widget == 6:
    st = os.path.dirname(os.path.realpath(__file__)) + '/../../scripts/stereo_package.sh'  
  subprocess.call([st])

class myMenu:

    # Obligatory basic callback
    def print_hello(self, w, data):
        print "Hello, World!"

    # This is the ItemFactoryEntry structure used to generate new menus.
    # Item 1: The menu path. The letter after the underscore indicates an
    #         accelerator key once the menu is open.
    # Item 2: The accelerator key for the entry
    # Item 3: The callback.
    # Item 4: The callback action.  This changes the parameters with
    #         which the callback is called.  The default is 0.
    # Item 5: The item type, used to define what kind of an item it is.
    #       Here are the possible values:

    #       NULL               -> "<Item>"
    #       ""                 -> "<Item>"
    #       "<Title>"          -> create a title item
    #       "<Item>"           -> create a simple item
    #       "<CheckItem>"      -> create a check item
    #       "<ToggleItem>"     -> create a toggle item
    #       "<RadioItem>"      -> create a radio item
    #       <path>             -> path of a radio item to link against
    #       "<Separator>"      -> create a separator
    #       "<Branch>"         -> create an item to hold sub items (optional)
    #       "<LastBranch>"     -> create a right justified branch 
    def add_item(self,item):
      self.menu_items.append(item)
      
    def get_main_menu(self, window):
      accel_group = gtk.AccelGroup()

      # This function initializes the item factory.
      # Param 1: The type of menu - can be MenuBar, Menu,
      #          or OptionMenu.
      # Param 2: The path of the menu.
      # Param 3: A reference to an AccelGroup. The item factory sets up
      #          the accelerator table while generating menus.
      item_factory = gtk.ItemFactory(gtk.MenuBar, "<main>", accel_group)

      # This method generates the menu items. Pass to the item factory
      #  the list of menu items
      item_factory.create_items(self.menu_items)

      # Attach the new accelerator group to the window.
      window.add_accel_group(accel_group)

      # need to keep a reference to item_factory to prevent its destruction
      self.item_factory = item_factory
      # Finally, return the actual menu bar created by the item factory.
      return item_factory.get_widget("<main>")

    def __init__(self):
        self.menu_items = [
            ( "/_File",         None,         None, 0, "<Branch>" ),
            ( "/File/_New",     "<control>N", self.print_hello, 0, None ),
            ( "/File/_Open",    "<control>O", self.print_hello, 0, None ),
            ( "/File/_Save",    "<control>S", self.print_hello, 0, None ),
            ( "/File/Save _As", None,         None, 0, None ),
            ( "/File/sep1",     None,         None, 0, "<Separator>" ),
            ( "/File/Quit",     "<control>Q", gtk.main_quit, 0, None ),
            ( "/_Packages",      None,         None, 0, "<Branch>" ),
            ( "/Web",           None,         None, 0, "<Branch>" ),
            ( "/_Help",         None,         None, 0, "<LastBranch>" ),
            ( "/_Help/About",   None,         None, 0, None ),
            ]
    def generate_menu(self,main_box,window):
        self.menubar = self.get_main_menu(window)
        main_box.pack_start(self.menubar, False, True, 0)
        self.menubar.show()