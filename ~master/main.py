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
def cb(s, t, p):
    global count
    if t == rt.ptype['DATA']:
        print p
    elif t == rt.ptype['JOIN']:
        print("Join from %04x." % (s))
        slaves.append(s)
    else:
        print "Packet from %x." % (s)

#C088
transport = rt(sys.argv[1], 9600, '\x00\x10', cb, sim_loss, 5)
transport.probe()
while True:
        print 'prepare'
        transport.prepare()
        time.sleep( 30 )
        for s in slaves:
          print 'poll'
          transport.poll(s)
          time.sleep(5)
transport.wait()
