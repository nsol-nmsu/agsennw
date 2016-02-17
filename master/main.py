#!/usr/bin/env python

# Xbee Transport
# Base station implementation

import time
from rtrans import rt
import struct
import sys
#from rapp import rapp_pkt

count = 0
sim_loss = 0
slaves = []
dat_size = 70; #int + float     
def cb(s, t, p):
    global count
    if t == rt.ptype['DATA']:
        for i in range(0 , len(p), dat_size):
            dat = struct.unpack('<20shh10s', p[i:i+dat_size])
            print "id: %d | address: %d | name: %s | value: %s" % dat
    elif t == rt.ptype['JOIN']:
        print("Join from %04x." % (s))
        slaves.append(s)

#C088
transport = rt(sys.argv[1], 9600, '\x10\x00', cb, sim_loss, 5)
transport.probe()
while True:
        for s in slaves:
          print 'poll'
          transport.poll(s)
          time.sleep(4)
transport.wait()
