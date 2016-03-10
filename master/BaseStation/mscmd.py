#!/usr/bin/env python

import time
import os
import socket

def finish():
        sock.close()
        quit()

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
                resp = sock.recv( 2048 )
                
                if resp == "":
                        print "Connection terminated by server"
                        finish()
                
                new = resp
                while not "\x03" in resp and not new == "" :
                        new = sock.recv( 2048 )
                        resp += new
                
                print resp[0:resp.find('\x03')]
except ( KeyboardInterrupt, EOFError ):
        finish()
