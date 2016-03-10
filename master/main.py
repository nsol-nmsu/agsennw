#!/usr/bin/env python

# Xbee Transport
# Base station implementation

import time
from serial import Serial
from mstrans import MSTrans
import struct
import sys
import os
import socket
import select
import threading

def joinCb( saddr, scount ):
        print( "JOIN" )
        if not saddr in slaves:
                slaves[saddr] = struct.unpack( '<B', scount )
        transport.accept( saddr )
        print slaves
        #transport.accept( saddr )

def segCb( saddr, segno, data ):
        print "SEGMENT", segno
        print "Source: ", struct.unpack( "<H", saddr )
        print data
        lines  = data.split("\n")
        header = lines[0].split(":")
        stype  = header[1]
        pin    = header[0]
        lines  = lines[1:-1]
        
        numaddr = struct.unpack( '<H', saddr )
        logname = workDir + "logs/" + str(numaddr[0]) + '_' + stype + '_' + pin + '.log'
        print logname
        logf = None
        print "Here"
        if not logname in logfiles:
                print "Not in logfiles"
                print "Opening"
                logf = open( logname, 'w' )
                print "Opened"
                logf.write( 'Hub Address: ' + str(numaddr[0]) + '\n' 
                                + 'Sensor: ' + stype + '(' + pin + ')\n' )
                logf.write( "Time (UTC), Time (Secs), Value Type, Value, Unit\n")
                print "Here"
                logfiles[logname] = logf
        
        print "Here"
        logf = logfiles[logname]
        
        print "Here"
        t = time.gmtime()
        print t
        tm = str(t.tm_year) + '(Y)|' + str(t.tm_mon) + '(M)|' + str(t.tm_mday) + '(D)|'\
                + str(t.tm_hour) + '(h)|' + str(t.tm_min) + '(m)|' + str(t.tm_sec) + '(s)'
        print tm
        for l in lines:
                cols = l.split(':')
                logf.write( tm + ', ' )
                logf.write( str( time.time() ) + ', ' )
                for c in cols:
                        logf.write( c + ',' )
                logf.write( '\n' )
        print "Here"

def handle_commands( commands ):
        for cmd in commands:
                cmd = cmd.split()
                switch = { "stop": lambda: stop(),
                           "cd"  : lambda: cd( cmd[1] ),
                           "sl"  : lambda: sl(),
                           "rm"  : lambda: rm( int(cmd[1]) ),
                           "add" : lambda: add( int(cmd[1]), int(cmd[2]) ),
                           "inv" : lambda: inv( int(cmd[1]) ),
                           "rst" : lambda: rst( int(cmd[1]) ),
                           "help" : lambda: help() }
                try:
                        switch[cmd[0]]()
                except (KeyError, IndexError, ValueError):
                        connection.send( "Invalid command format.\n" )
                        help()

def stop():
        global servLock
        for k in logfiles:
                logfiles[k].close()
        
        servLock.release()
        serveThread.join()
        transport.kill()
        quit()
def cd( d ):
        global workDir
        if os.path.exists( d ):
                workDir = d
                connection.send( "Done\n" )
        else:
                connection.send( "Error: directory does not exists\n" )

def sl():
        if len( slaves ) == 0:
                connection.send( "None\n" )
        else:
                for s in slaves:
                        addr = str(struct.unpack( '<H', s ))
                        cconnection.send( addr  + ": " + str(slaves[s]) )

def rm( slave ):
        addr = struct.pack( '<H', slave )
        if addr in slaves:
                del slaves[addr]
                connection.send( "Done\n" )
        else:
                connection.send( "No such slave\n")

def add( slave, scount ):
        addr = struct.pack( '<H', slave )
        slaves[addr] = scount
        connection.send( "Done\n" )

def inv( slave ):
        addr = struct.pack( '<H', slave )
        transport.invite( addr )
        connection.send( "Done\n" )

def rst( slave ):
        addr = struct.pack( '<H', slave )
        if addr in slaves:
                transport.reset( addr )
                rm( slave )
                connection.send( "Done\n" )
        else:
                connection.send( "No such slave\n" )

def help():
        connection.send("""
exit                 -- Exit mscmd
cd [path]            -- Change working and logging directory
sl                   -- List slaves
rm [slave]           -- Remove a slave
add [slave][scount]  -- Force add a slave with scount sensors
inv [slave]          -- Invite a slave to join
rst [slave]          -- Reset a slave, must re-join
help                 -- Print this help page""")

class ConReader(threading.Thread):
        def __init__(self, con):
                threading.Thread.__init__(self)
                self.connection = con
                self.commands   = []
                self.alive      = True
                self.stopLock   = threading.Lock()
                self.stopLock.acquire()
        def run(self):
                while not self.stopLock.acquire(False):
                        dat = self.connection.recv( 512 )
                        if dat == "":
                                break;
                        self.commands += dat.split( "\n" )
                        print "received", dat
                self.alive = False
                self.connection.close()
        def kill():
                self.stopLock.release()

def servFunc():
        global connections
        #Setup command server
        if not os.path.exists(homeDir + "/.msnet"):
                os.makedirs(homeDir + "/.msnet")
        cmd_sock_addr  = homeDir + "/.msnet/msnet_cmd.sock"
        cmd_sock = socket.socket( socket.AF_UNIX, socket.SOCK_STREAM )
        
        try:
                cmd_sock.bind( cmd_sock_addr )
        except:
                print "Error: binding socket", sys.exc_info()[0]
                print "file", cmd_sock_addr, "may be broken, delete this file and retry."
                return
        try:
                print "created", cmd_sock_addr
                
                while not servLock.acquire(False):
                                cmd_sock.listen(1)
                                con = cmd_sock.accept()[0]
                                print "accepted"
                                conLock.acquire(True)
                                try:
                                        connections.append( ConReader( con ) )
                                        connections[-1].start()
                                finally:
                                        conLock.release()
                                print "here"
        except Exception, e:
                print e
        finally:
                print "Closed"
                conLock.acquire(True)
                for c in connections:
                        c.kill()
                connections = []
                conLock.release()
                
                cmd_sock.close()
                os.remove(cmd_sock_addr)
        

#Env
workDir = "."
homeDir = os.path.expanduser("~")
slaves = []
logfiles = {}
connections = []
conLock    = threading.Lock()
servLock   = threading.Lock()


#Setup
try:
        tty = Serial(sys.argv[1], 9600)
        transport = MSTrans( tty, "127", joinCb, segCb )
        servLock.acquire()
        serveThread = threading.Thread( target=servFunc )
        serveThread.start()
        

        transport.invite("\xFF\xFF")
        while True:
                transport.user( "\xff\xff", "\x00" )
                time.sleep( 10 )
                for s in slaves:
                        print "requested from ", s
                        transport.request( slaves[s], [] )
                        time.sleep( 5 )
                if conLock.acquire(False):
                        try:
                                for c in connections:
                                        handle_command( c.command )
                        finally:
                                conLock.release()
except KeyboardInterrupt:
        stop()
        quit()
