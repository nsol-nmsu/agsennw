#!/usr/bin/env python

# Xbee Transport
# Base station implementation

import time, re
import sys
import os
import socket
import select
import threading



"""Class to manage a single connection, reads on it's own
thread calls a callback function for each command read.  Also
transmits data passed to it."""
class ConManager(threading.Thread):
        def __init__(self, con, callback):
                threading.Thread.__init__(self)
                self.messages   = []
                self.connection = con
                self.callback   = callback
                self.stopLock   = threading.Lock()
                print "MADE"
        
        def start( self ):
                self.stopLock.acquire()
                threading.Thread.start( self )
        def run(self):
            print "STARTED"
            try:
                while not self.stopLock.acquire(False):
                        
                        print "H1"
                        #Send all queued messages
                        for m in self.messages:
                                self.connection.send( m )
                        
                        self.messages = []
                        
                        print "H2"
                        try:
                                #Receive a message
                                self.connection.settimeout( 5.0 )
                                dat = self.connection.recv( 512 )
                                print "RECEIVED"
                        except socket.timeout:
                           continue
                        
                        #Get rid of EOF, or other undesireable char.  Was causing an index error.
                        dat = dat[0:-1]
                        
                        print "H3"
                        #If message is "" then connection is closed, this manager is done
                        if dat == "":
                                break
                        
                        print "H4"
                        #Handle callbacks
                        commands = dat.split( "\n" )
                        for c in commands:
                                self.callback( self, c )
                        
                        
            except Exception, e:
                print e
            finally:
                #Clean up
                self.connection.close()
                
        def kill( self ):
                try:
                        self.stopLock.release()
                except Exception, e:
                        print e
        def post( self, message ):
                try:
                        self.messages.append( message )
                except Exception, e:
                        print e

"""Allows command registration and generates a list of actions to perform"""
class CmdServer(threading.Thread):
        def __init__(self, addr, defcmd ):
                threading.Thread.__init__( self )
                try:
                        self.addr = addr;
                        self.killLock = threading.Lock()
                        self.commands = {}
                        self.defcmd = defcmd
                        self.todo =  []
                        self.connection = None
                except Exception, e:
                        print "Error: ", e
                        raise
        
        def add( self, command, action ):
                try:
                        self.commands[command] = action
                except Exception, e:
                        print e
        
        def add_all( self, commands ):
                try:
                        for c in commands:
                                self.commands[c] = commands[c]
                except Exception, e:
                        print e
        
        def start( self ):
                try:
                        self.killLock.acquire()
                        threading.Thread.start( self )
                except Exception, e:
                        print e
        
        def run( self ):
                try:
                        sock = socket.socket( socket.AF_UNIX, socket.SOCK_STREAM )
                        try:
                                if os.path.exists( self.addr ):
                                        os.remove( self.addr )
                        except Exception, e:
                                print "Some other application is using my address: ", self.addr
                                print e
                                sock.close()
                                return
                                
                        sock.bind( self.addr )
                        
                        try:
                                print "cmd1"
                                while not self.killLock.acquire(False):
                                        if self.connection == None:
                                        
                                            #Give the socket a timeout
                                            try:
                                                sock.settimeout( 5.0 )
                                                sock.listen(1)
                                                con = sock.accept()[0]
                                                print "ACCEPTED"
                                                self.connection = ConManager( con,
                                                                         self._post_action )
                                                self.connection.start()
                                            except socket.timeout:
                                                continue
                                        elif not self.connection.isAlive():
                                                print "cmd3"
                                                self.connection = None
                                        else:
                                                print "cmd4"
                                                time.sleep( 1 )
                        finally:
                                print "cmd5"
                                if not self.connection == None:
                                        self.connection.kill()
                                        self.connection.join()
                                print "cmd6"
                                sock.close()
                                os.remove(self.addr)
                
                except Exception, e:
                        print e
        
        def kill( self ):
                print "Killing CmdServer"
                self.killLock.release()
                print "Killed CmdServer"
        
        def _post_action( self, conman, command ):
                try:
                        parts = command.split()
                        try:
                                self.todo.append( ( self.commands[parts[0]],
                                                         conman, parts[1:] ) )
                                
                        except IndexError, e:
                                self.todo.append( ( self.commands[parts[0]], conman, [] ) )
                        except KeyError, e:
                                conman.post( "Unknown command\"");
                                self.todo.append( (self.defcmd, conman, parts ) )
                except Exception, e:
                        print e
        
        def do_action( self ):
                try:
                        cmd = self.todo[0][0]
                        con = self.todo[0][1]
                        args = self.todo[0][2]
                        
                        ret = cmd( args )
                        con.post( ret )
                        del self.todo[0]
                except Exception, e:
                        print e

        def do_all( self ):
                try:
                        for t in self.todo:
                                self.do_action()
                except Exception, e:
                        print e
                        
                
