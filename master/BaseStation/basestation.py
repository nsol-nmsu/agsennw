#!/usr/bin/env python

# Xbee Transport
# Base station implementation

import time
from serial import Serial
from mstrans import MSTrans
import struct, sys, os, socket, threading
from cmd_server import CmdServer

#MSTrans callbacks
def joinCb( saddr, scount ):
        print "Join from", hex( struct.unpack( "<H", saddr )[0] )
        try:
                scount = struct.unpack( '<B', scount[0:1] )[0]
                if scount > 0:
                        slaves[saddr] = scount
                        trans.accept( saddr )
        except Exception, e:
                print e
        return

def segCb( saddr, segno, data ):
        global pending_segments
        global segments_lock
        global logfiles
        
        
        try:
                segments_lock.acquire()
                if segno in pending_segments:
                        del pending_segments[segno]
                else:
                        #Don't want duplicates
                        return
        except Exception, e:
                print e
        finally:
                segments_lock.release()

        lines  = data.split("\n")
        header = lines[0].split(":")
        stype  = header[1]
        pin    = header[0]
        lines  = lines[1:-1]
        
        #Manage logging operations
        naddr   = struct.unpack( "<H", saddr )
        
        if not os.path.exists( wdir + "/logs" ):
                os.makedirs( wdir + "/logs" )
        logname = wdir + "/logs/" + str( naddr[0] ) + "_" + stype + "_" + pin + ".log"
        
        try:
                logs_lock.acquire()
                #If files arent already added then add them
                if not logname in logfiles:
                        logf = open( logname, "w" )
                        
                        #Put header
                        logf.write( "Time (UTC), Time (Secs), Value Type, Value, Unit\n")
                        logfiles[logname] = logf
                
                logf = logfiles[logname]
                
                now = time.gmtime()
                formated = str( now.tm_year ).zfill( 4 ) + "-" + str( now.tm_mon ).zfill(2) \
                                + "-" + str( now.tm_mday ).zfill( 2 ) + "|" \
                                + str( now.tm_hour ).zfill( 2 ) \
                                + ":" + str( now.tm_min ).zfill( 2 ) + ":"\
                                + str( now.tm_sec ).zfill( 2 )
                for l in lines:
                        cols = l.split( ":" )
                        logf.write( formated + ", " )
                        logf.write( str( time.time() ) + ", " )
                        for c in cols:
                                logf.write( c + ", " )
                        logf.write( "\n" )
        finally:
                logs_lock.release()
        return
        
#CmdServer callbacks
def killCb( params ):
        global stop_loop
        
        print "Shutting Down..."
        stop_loop = True
        
        print "Killing CmdServer..."
        cmdserver.kill()
        cmdserver.join()
        
        print "Killing MSTrans..."
        trans.kill()
        
        print "Closing Log Files..."
        for f in logfiles:
                logfiles[f].close()
        
        print "Done"
        quit()
        
def cdCb( params ):
        global logfiles
        if len( params ) == 0:
                return "WD is " + os.getcwd() + "\n"
        
        path = os.path.expanduser( params[0] )
        if path[-1] == "/":
                path = path[0:-1]
        
        try:
                if not os.path.exists( path ):
                        os.makedirs( path )
        except:
                return "Could not create directory"
        
        if os.path.isdir( path ):
                try:
                        logs_lock.acquire()
                        wdir = path
                        for f in logfiles:
                                logfiles[f].close()
                        logfiles = {}
                finally:
                        logs_lock.release()
                return "New WD is " + path + "\n"
        else:
                return "Invalid path\n"

def to_addr( s ):
        try:
                if s[0:2] == "0x":
                        return struct.pack( "<H", int( s, 16 ) )
                elif s[0:2] == "0c":
                        return struct.pack( "<H", int( s, 8 ) )
                elif s[0:2] == "0b":
                        return struct.pack( "<H", int( s, 2 ) )
                else:
                        return struct.pack( "<H", int( s ) )
                        
        except ValueError:
                return None

def slCb( params ):
        ret = ""
        for s in slaves:
                naddr = struct.unpack( "<H", s )[0]
                if len(params) > 0:
                        if params[0] == "x":
                                strep = hex( naddr )
                        elif params[0] == "o":
                                strep = oct( naddr )
                        elif params[0] == "b":
                                strep = bin( naddr )
                        else:
                                strep = str( naddr )
                else:
                        strep = str( naddr )
                ret += strep + "(" + str(slaves[s]) + ")\t" 
        ret += "\n"
        return ret
def invCb( params ):
        global trans
        a = to_addr( params[0] )
        if a:
                trans.invite( a, [] )
                return "Invite Done\n"
        else:
                return "Invalid address\n"
def rmCb( params ):
        global slaves
        a = to_addr( params[0] )
        if a:
                if a in slaves:
                        del slaves[a]
                        return "Remove Done\n"
                else:
                        return "No such slave\n"
        else:
                return "Invalid address\n"
def addCb( params ):
        global slaves
        
        a = to_addr( params[0] )
        
        try:
                c = int( params[1] )
        except IndexError:
                return "Must specify the hub's slave count\n"
        except ValueError:
                return "Invalid slave count, must be decimal integer\n"

        if not a == None:
                if a in slaves:
                        return "Slave already joined\n"
                else:
                        slaves[a] = c
                        return "Add Done\n"
        else:
                return "Invalid address\n"

def rstCb( params ):
        global trans
        a = to_addr( params[0] )
        if a:
                trans.reset( a, [] )
                return "Reset Done\n"
        else:
                return "Invalid address\n"
def configCb( params ):
        return "Config is not implemented yet\n"
def modeCb( params ):
        if params[0] == "inter":
                interactive = True
                return "Mode Change Done\n"
        elif params[1] == "norm":
                interactive = False
                return "Mode Change Done\n"
        else:
                return "Unknown option\n"

def helpCb( params ):
        return """[X] Indicates an unimplemented command or option
kill             -- Kill the server
cd    <path>     -- Change working directory
                        *this is where the logs are sent
sl               -- List slaves and their details
inv   <addr>     -- Invite a slave to join the network
                        *addr 0xFFFF is broadcast
rm    <addr>     -- Force remove a slave from the network
add   <addr> <s> -- Force add a slave to the network
                        *<s> is slave sensor count
reset <addr>     -- Request a slave reset
                        *addr 0xFFFF is broadcast
config [opts...] -- [X]Change some configuration
mode inter|norm  -- Switch between interactive and normal operation.
                        *in interactive this terminal's instructions are executed
                         instantly ( or close enough ).
                        *in normal mode the server resumes normal data collection
                         operations and some operatiosn ( deemed unclean ) can cause
                         collection problems if executed immediately so they are queued
                         for execution in the next loop iteration.\n"""

#Env
interactive     = False
wdir            = "."
home            = os.path.expanduser("~")
logs_lock        = threading.Lock()
logfiles        = {}
slaves          = {}
sample_period   = 10
prepare_period  = 10
request_timeout = 5
retry_max       = 5
pending_segments = []
segments_lock   = threading.Lock()
stop_loop       = False


#Setup
cmdserver = CmdServer( os.path.expanduser("~") + "/.msnet/msnet_cmd.sock", helpCb )
commands = {
                "kill"  : ( killCb, False),
                "cd"    : ( cdCb, True ),
                "sl"    : ( slCb, True ),
                "inv"   : ( invCb, False ),
                "rm"    : ( rmCb, False ),
                "add"   : ( addCb, False ),
                "rst"   : ( rstCb, False ),
                "config": ( configCb, False ),
                "mode"  : ( modeCb, False ),
                "help"  : ( helpCb, True )   }
cmdserver.add_all( commands )

tty = Serial( sys.argv[1], 9600 )
trans = MSTrans( tty, "127", joinCb, segCb )
cmdserver.start()

print "Running..."

#Basic operations loop
while not stop_loop:
        try:
                #Normal mode exclusive operations
                if not interactive:
                        trans.invite("\xFF\xFF")
                        trans.user( "\xFF\xFF", "\x00" )
                        time.sleep( prepare_period )
                        for s in slaves:
                            try:
                                segments_lock.acquire()
                                pending_segments = range( 0, slaves[s] )
                                for i in range( 0, retry_max ):
                                        if len( pending_segments ) == 0:
                                                break
                                        trans.request( s, pending_segments )
                                        time.sleep( request_timeout )
                            finally:
                                segments_lock.release()
                        if sample_period > prepare_period:
                                time.sleep( sample_period - prepare_period )
                
                #Mode common operations
                cmdserver.do_all()
        except KeyboardInterrupt:
                break

killCb( [] )
