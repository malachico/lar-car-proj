import serial
import sys
import thread
import time


def print_time( threadName, delay):
   count = 0
   print "%s: %s" % ( threadName, time.ctime(time.time()) )
   while True:
      time.sleep(delay)
      st = raw_input("Enter your input: ")
      st = 'M'+st+',+062000'
      ser.write(st)
      

ser = serial.Serial('/dev/ttyUSB0', 9600, timeout=0)
ser.open()
print ("connected to: " + ser.portstr)
count=0

try:
    thread.start_new_thread( print_time, ("Thread-1", 2, ) )
    while True:
        line =ser.readline() # should block, not take anything less than 14 bytes
        if line:
            # Here I want to process 14 bytes worth of data and have
            # the data be consistent.
            sys.stdout.write(line)
            count=count+1
finally:
    ser.close()
    print "goodbye"
