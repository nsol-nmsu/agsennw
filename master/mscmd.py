#!/usr/bin/env python

import time
import os
import socket

workDir = "./"

#Setup socket
try:
        sock = socket.socket( socket.AF_UNIX, socket.SOCK_STREAM )
        sock.connect( os.path.expanduser("~") + "/.msnet/msnet_cmd.sock" )
except socket.error, e:
        print "Error: ", e
        print "Make sure that server is running and no other clients are connected"
        quit()

try:
        while True:
                sock.send( raw_input( "?>" ) )
                sock.settimeout( 5.0 )
                print sock.recv( 512 )
except KeyboardInterrupt:
        sock.close()
        quit()
