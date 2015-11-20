#!/usr/bin/env python

from rt_pkt import rt_pkt
from serial import Serial
from xbee import XBee
import time, struct, threading, random
        
# transport layer implementation
class rt:

    # type field constants
    ptype = { 'ACK':   254,
              'NAK':   255,
              'PROBE': 0,
              'JOIN':  1,
              'POLL':  2,
              'DATA':  3,
              'SET':   4,
              'ERR':   5
            }

    def __init__(self, tty, baud, addr, callback, loss=0.0, probe_time=5):
        self.tty  = Serial(tty, baudrate=baud)
        self.xbee = XBee(self.tty, callback=self._recv_frame)
        self.addr = struct.unpack("<H", addr)[0]
        print self.addr
        self._frame = 0
        self._data = {}
        self._timer = {}
        self._waiting = {}
        self._callback = callback
        self._loss = loss
        self._probe_time = probe_time
    
    def _send(self, dest, pkg_type, pkg_no, seg_ct, seg_no, payload):
        rp = { 'master':   self.addr,
               'slave':    dest,
               'pkg_no':   pkg_no,
               'pkg_type': pkg_type,
               'seg_ct':   seg_ct,
               'seg_no':   seg_no,
               'payload':  payload
             }
        if random.random() > self._loss:
            self.xbee.tx(dest_addr=struct.pack(">H", dest), data=rt_pkt(parms=rp).raw)
            s = rt_pkt(parms=rp).raw

    def _ack(self, pkt):
        #print("ACKing %04x/%d.%d" % (pkt['slave'], pkt['pkg_no'], pkt['seg_no']))
        self._send(pkt['slave'], rt.ptype['ACK'], pkt['pkg_no'], 1, pkt['seg_no'], "")
    
    def send(self, dest, pkg_type, payload):
        self._send(dest, pkg_type, self._frame, 1, 0, payload)
        self._frame += 1
        
    def wait(self):
        while True:
            try:
                time.sleep(0.1)
            except KeyboardInterrupt:
                self._end()
                time.sleep(0.1)
                break
                
    def _ptimer(self, pid, delay=5.0, cb=None):
        if cb == None:
            cb = rt._flow_expire
        if pid in self._timer:
            self._timer[pid].cancel()
       #print("starting timeout timer for %s, callback is %s" % (pid, cb))
        self._timer[pid] = threading.Timer(delay, cb, [self, pid])
        self._timer[pid].start()
                
    def _flow_expire(self, pid):
        print("Flow %04x/%d expired" % pid)
        del self._data[pid]
        del self._timer[pid]
        
    def _poll_retx(self, pid):
        del self._timer[pid]
        self.poll(pid)
        
    def _recv_frame(self, x):
        if x['id'] == 'rx':
            if random.random() > self._loss:
                self._proc_frame(x)
    
    last_pkt = -1;
    def _proc_frame(self, x):  
        if x['id'] == 'rx':
            
            # parse incoming packet
            pkt = rt_pkt(raw=x['rf_data'])
            
                    
            # ack the packet
            self._ack(pkt)                    
                    
            # if the packet was a join, poll the node for data
            if pkt['pkg_type'] == rt.ptype['JOIN']:
                self._slaves[pkt['slave']] = pkt['slave']
                        
            # handle data packet
            elif pkt['pkg_type'] == rt.ptype['DATA']:
                print("Got data segment %04x/%d.%d" % (pkt['slave'], pkt['pkg_no'], pkt['seg_no']))
                pid = (pkt['slave'], pkt['pkg_no'])
                
                # check if we were waiting for this slave's data
                if pkt['seg_no'] == 0 and pkt['slave'] in self._waiting:
                    self._timer[pkt['slave']].cancel()
                    del self._timer[pkt['slave']]
                    del self._waiting[pkt['slave']]
                
                # add segment to the corresponding buffer and call the callback if it is complete
                if pkt['seg_no'] == 0 and pkt['seg_ct'] == 1:
                    #print("first and only packet")
                    self._callback(pkt['slave'], pkt['pkg_type'], pkt['payload'])
                elif pkt['seg_no'] == 0:
                    #print("first but not only packet")
                    self._data[pid] = [pkt]
                    self._ptimer(pid)
                elif pid in self._data:
                    #print("not first packet")
                    self._timer[pid].cancel()
                    l = self._data[pid]
                    if l[len(l)-1]['seg_no'] == pkt['seg_no'] - 1:
                        #print("this is the packet we were waiting for")
                        l.append(pkt)
                        if pkt['seg_no'] == pkt['seg_ct'] - 1:
                            print("we got the last segment")
                            payload = "".join([pp['payload'] for pp in l])
                            self._callback(pkt['slave'], pkt['pkg_type'], payload)
                            del self._data[pid]
                    else:
                        #print("the packet we received wasn't one we were waiting for")
                        self._ptimer(pid)
                else:
                    print("[rt] unexpected segment %04x/%d.%d" % (pkt['slave'], pkt['pkg_no'], pkt['seg_no']))
                
    def probe(self):
        self._slaves = {}
        for i in range(0, int(2*self._probe_time)):
            print("Sending probe (%d)..." % i)
            self.send(0xffff, rt.ptype['PROBE'], "")
            time.sleep(1)
        for i, v in self._slaves.items():
            self._callback(i, rt.ptype['JOIN'], "")
        
    def poll(self, addr):
        self._waiting[addr] = addr
        self._ptimer(addr, delay=2.0, cb=rt._poll_retx)
        #print("Sending poll to %04x" % addr)
        self.send(addr, rt.ptype['POLL'], "")
    
    def _end(self):    
        self.xbee.halt()
        self.tty.close()

