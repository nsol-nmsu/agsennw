# Agriculture Sensor Network Master

This directory contains the files needed to implement the network base
station.  The base station collects and logs sensor data from it's
connected hubs.

## class MSTrans:
This class implements XBee communication with the sensor hubs.  The class resides in the mstrans.py file and it's simple communication protocol can be found [here](#communication-protocol-master-perspective-).
   * [MSTrans( tty, net, joinCb, segCb )](#mstrans-tty-net-joincb-segcb-)
   * [kill()](#kill-)
   * [invite( slave, info )](#invite-slave-info-)
   * [accept( addr )](#accept-addr-)
   * [request( addr, segs )](#request-addr-segs-)
   * [user( addr, action )](#user-addr-action-)

---

##### MSTrans( tty, net, joinCb, segCb )
MSTrans class constructor.  Configures the connected XBee unit and
registers the provided callbacks.

###### tty
a Serial object from pySerial to use for communication with the XBee.


###### net
the network id we should assign to the XBee, for now this should be "127" since it's hardcoded into the hubs.

###### joinCb
a callback to call when MSTrans receives a join request, the user will need to accept the request calling MSTrans.accept(), otherwise the request will be refused. This callback is passed 2 parameters ( saddr, scount ).  'saddr' is a 2-byte string representing the joining slave's address, and 'scount' is an integer representing number of segments the slave has available.

###### segCb
a callback to call when MSTrans receives a segment from a hub.  This function will be passed 3 parameters ( sadder, segno, data ).  'saddr' is the address of the slave that sent the segment, 'segno' is the segment's index number, and 'data' is a byte string of the segment's data.

---

##### kill()
Halts the server thread and cleans up.  Application will have difficulty exiting while MSTrans thread is running, so this should be called before attempting to exiting the user program.

---

##### invite( slave, info )
Invites a slave to join the network, the slave will be sent an INVITE packet, to which it may respond with a JOIN request that must be handled by the user provided [joinCb](#joincb).

###### slave
The 2-byte string network address of the slave to invite

###### info
A byte string containing some kind of info about the master that the slave may use to decide whether to join or not.

---

##### accept( addr )
Sends an ACCEPT packet to the slave at `addr` telling it that it's join request has been accepted.

###### addr
Address of the slave to accept.

---

##### request( addr, segs )
Request some number of segments from the slave, when the segments are received they'll be passed to the user provided [segCb](#segcb).

###### addr
The address of the slave to request segments of.

###### segs
A list of segment numbers (integers) representing the segments to request.

---

##### user( addr, action )
Sends a USER packet to the slave at `addr` which is handled by a user implemented callback on the slave side.  The user packet takes an `action` parameter which is a byte string containing any data that should be passed to the callback on the slave side.

###### addr
The address of the slave that should be sent a USER packet.

###### action
A byte string containing the data that should be passed to the user callback on the slave side.

---

## Addressing
MSTrans assigns the address of 0x00 to the master of the network and the network nodes are expected to have addresses 0x01-0xFF.


## Communication Protocol (Master Perspective)
The communication protocol used by MSTrans is extremely simple, and probably a bit lacking in terms of robustness, but it's lightweight and works for it's assigned purpose.  The protocol is mostly for one way communication, with the master polling the slave for data.  However it also features a simple means of master to slave communication for configuration and other necessary actions.  Overall the protocol supports 4 communication actions.

### invite
The invite action invites a slave at address x to the master's network.  This action should be performed before any others, since the slave shouldn't respond to a network that it isn't joined with and so wouldn't respond to any other actions until it has been invited and accepted by the master.

INVITE:
[ 1: packet type = 5, 1: checksum, $: variable length byte string of master info ]

### accept
After receiving an invitation, the slave will respond with a join request.  At this point the master user must respond with an accept action, otherwise the slave will be unable to join the network.

ACCEPT:
[ 1: packet type = 6, 1: checksum ]

### request
The basic unit of communication between the master and slave is a segment.  Each slave has a fixed number of segments in which they can store data that they wish to communicate with the master.  The master can then retrieve any number of the segments with a GET request.  Any of the slave's segments can be requested at any time in any order.

GET:
[ 1: packet type = 3, 1: checksum, 1: number of segments,
$:byte array of segment numbers ]

### user
The user action is MSTrans means of master to slave communication.  This action sends a set of data to the user callback registered with the slave, and is handled in by the callback in some way depending on it's implementation.

USER:
[ 1: packet type = 2, 1: checksum,
$: byte array of information to be handled by user callback]

---

## class CmdServer
This class implements a simple unix socket listener for command line control of the base station.  The class allows users to register commands and their callbacks, and to flag whether the operation will affect the state of the basestation.  If the action will affect the state of the basesation then it is added to a queue and preformed when the user calls [do_action()](#do_action-) of [do_all()](#do_all-).  This class resides in the cmd_server.py file.

   * [CmdServer( addr, defcmd )](#cmdserver-addr-defcmd-)
   * [add( command, action, clean )](#add-command-action-clean-)
   * [add_all( commands )](#add_all-commands-)
   * [start()](#start-)
   * [kill()](#kill-)
   * [do_action()](#do_action-)
   * [do_all()](#do_all-)

---

##### CmdServer( addr, defcmd )
Initializes state and socket address as well as the default command.

###### addr
The address to use for communication with terminal.

##### defcmd
The default command to run when the user enters an unregistered command or invalid syntax.  This is a callback that takes a list of parameters as a single argument.  CmdServer considers this command to be clean, that is it does not affect the base station's state, and so executes it immediately when requested.

---

##### add( command, action, clean )
Adds a single command to CmdServer's listening pool.

###### command
A string representing the command text, when cmdServer receives this text from a connected terminal it will execute 'action'

###### action
A callback of the form callback( params ), where 'params' is a list of strings representing the parameters passed to the command by the user.  The callback should return a string, which will then be sent to the connected terminal.

###### clean
A boolean value stating whether this action executes cleanly, without affecting the state of the basestation, or not.  If this is False then the action will not execute immediately when requested, it will be executed when do_action() or do_all() are called.  Otherwise the action will be executed immediately and it's returned string will be returned to the connected terminal.

---

##### add_all( commands )
Calls add() on each of the entries in the 'commands' dictionary.

###### commands
List of commands to be added to the CmdServer, command entries are of the form
{"command_name" : (action, clean)}

---

##### start()
Starts the command listening thread, the CmdServer will not function until this method is called.

---

##### kill()
Stops the CmdServer listener thread, exiting python will be difficult without calling this function if start() was previously called.

---

##### do_action()
Executes the action on top of the non-clean commands queue.

---

##### do_all()
Executes all actions in the non-clean commands queue.

---

## basestation.py
This file puts both MSTrans and CmdServer together into a usable basestation.  The implementation is horrible since it is the first prototype and was written in a hurry.  The basestation.py application itself should be rewritten at some point because of it's horrid state.

Basically the program just sits in a loop and polls all joined slaves every 5 mins.  When it receives data from a slave it logs it in a file with the naming format:

slaveaddr_slavetype_slavepin

Where slaveaddr is the slave's network address, slavetype is the slave's typestring (mime type?), and slavepin is the pin on the hub that the slave is plugged into.
