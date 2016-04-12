#!/usr/bin/env python

from serial import Serial
from xbee import XBee
from xbee.helpers.dispatch import Dispatch
from threading import Lock
from threading import Thread
import time, struct

class MSTrans:
        ptype = {
                'JOIN'    : 0,
                'SEGMENT' : 1,
                'USER'    : 2,
                'GET'     : 3,
                'OK'      : 4,
                'INVITE'  : 5,
                'ACCEPT'  : 6 }

        def __init__( self, tty, net, joinCb, segCb ):
                self.tty    = tty
                self.joinCb = joinCb
                self.segmentCb  = segCb
                self.net    = net
                self._xbee_lock = Lock()
                
                xbee_inits1 = [  "+++",
                                "ATRE\r",
                                "ATFR\r"]
                xbee_inits2 =  [
                                "+++",
                                "ATCH0C\r",
                                "ATID" + self.net +"\r",
                                "ATAP1\r",
                                "ATAC\r"]
                print( "Configuring XBee" )
                for cmd in xbee_inits1:
                        self.tty.write( cmd )
                        resp = tty.read(3)
                        print( cmd[0:-1] + " " + resp )
                        if not "OK" in resp:
                                print( "Error setting up XBee" )
                                quit()
                time.sleep(1)
                self.tty.flushInput()
                for cmd in xbee_inits2:
                        self.tty.write( cmd )
                        resp = tty.read(3)
                        print( cmd[0:-1] + " " + resp )
                        if not "OK" in resp:
                                print( "Error setting up XBee" )
                                quit()
                self.tty.write("ATCN\n");
                
                time.sleep(10)
                self.xbee = XBee( tty, callback=self._xbee_cb )
                
        def kill(self):
                self.xbee.halt()
        def _xbee_cb( self, d ):
                try:
                        if d['id'] == 'rx':
                                data   = d['rf_data']
                                source = d['source_addr']
                                typ    = struct.unpack( "<B", data[0:1] )[0]
                                csum   = struct.unpack( "<B", data[1:2] )[0]
                                
                                #Verify checksum
                                sm = 0
                                fmt = "<%dB"%len( data )
                                bytes = struct.unpack( fmt, data )
                                for b in bytes:
                                        sm = 0xFF & ( sm + b )
                                if not sm == 0xFF:
                                        print "Bad checksum", sm
                                        return
                                

                                if typ == self.ptype['JOIN']:
                                        if len( data ) > 2:
                                                self._handle_join( source, data[2:] )
                                        else:
                                                self._handle_join( source, [] )
                                elif typ == self.ptype['SEGMENT']:
                                        if len( data ) > 2:
                                                self._handle_seg( source, data[2:] )
                                        else:
                                                self._handle_seg( source, [] )
                except Exception, e:
                        print e
        def _handle_join( self,  s, d ):
                self.joinCb( s, d )
        def _handle_seg( self, s, d ):
                segnum = struct.unpack('<B', d[0] )[0]
                self.segmentCb( s, segnum, d[1:] )
                
        def invite( self, slave, info ):
                try:
                        data = struct.pack( "<BB%ds"%len(info), self.ptype['INVITE'], 0, info )
                        data = data[0:1] + self._csum( data ) + data[2:] 
                        self.xbee.tx( dest_addr=slave, data=data )
                except Exception, e:
                        print e

        def request( self, addr, segs ):
                try:
                        if len(segs) > 0:
                                fmt = "BBB%uB" % len(segs)
                                packet = struct.pack( fmt, self.ptype['GET'], 0, len( segs ), *segs )
                        else:
                                packet = struct.pack( "<BBB", self.ptype['GET'], 0, 0 )
                        packet = packet[0:1] + self._csum( packet ) + packet[2:] 
                        self.xbee.tx( dest_addr=addr, data=packet )
                except Exception, e:
                        print e
        def accept( self, addr ):
                try:
                        packet = struct.pack('<BB', self.ptype['ACCEPT'], 0 )
                        packet = packet[0:1] + self._csum( packet ) + packet[1:]
                        self.xbee.tx( dest_addr=addr, data=packet )
                except Exception, e:
                        print e

        def user( self, addr, action ):
                try:
                        packet = struct.pack('<BB%ds'%len(action), self.ptype['USER'], 0, action )
                        packet = packet[0:1] + self._csum( packet ) + packet[2:]
                        self.xbee.tx( dest_addr=addr, data=packet )
                except Exception, e:
                        print e

        def _csum( self, p ):
                try:
                        sm = 0
                        fmt = "<%dB"%len(p)
                        bytes = struct.unpack( fmt, p )
                        for b in bytes:
                                sm = 0xFF & ( sm + b )
                                
                        return struct.pack( "<B", 0xFF - sm )
                except Exception, e:
                        print e

