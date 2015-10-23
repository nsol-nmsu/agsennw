#!/usr/bin/env python

# Xbee Transport
# Base station implementation

import time
from rtrans import rt
import struct
#from rapp import rapp_pkt

count = 0
sim_loss = 0
slaves = []
dat_size = 6; #int + float     
def cb(s, t, p):
    global count
    if t == rt.ptype['DATA']:
        print("got a packet from %02x, length %d" % (s, len(p)))
        for i in range(0 , len(p), dat_size):
          if i < len(p):
            dat = struct.unpack('<hf', p[i:i+dat_size])
            print "%d : address: %x , id: %x , data: %f" % (count, s, dat[0] , dat[1])
            count += 1
    elif t == rt.ptype['JOIN']:
        print("Join from %04x." % (s))
        slaves.append(s)

#C088
transport = rt("/dev/ttyUSB0", 9600, '\x10\x00', cb, sim_loss, 5)
transport.probe()
while True:
  for s in slaves:
    transport.poll(s)
    time.sleep(2)
transport.wait()
