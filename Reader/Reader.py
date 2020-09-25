#!/usr/bin/env python

# Install first pyserial: 'python pip install pyserial'

import serial, signal, sys, time
from serial import Serial

arduino = serial.Serial('COM3', 115200)

def signal_handler(sig, frame):
    arduino.close()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)
while True:
    cadena = arduino.readline()
  
    if(cadena.decode() != ''):
      print(cadena.decode(), end = '')
  
    time.sleep(0.1)
