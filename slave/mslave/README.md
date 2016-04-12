# Agriculture Sensor Network Hub

This directory contains the files needed to implement the network sensor hub code.

## file mtrans.h
This file provides the mstrans namespace that implements the hub's communication
with it's master.

   * [int mstrans.init( SoftwareSerial& ss, InviteCb invcb, UserCb usercb )](#int-mstrans-init-softwareserial-ss-invitecb-invcb-usercb-usercb-)
   * [int mstrans.add_segment( int len )](#int-mstrans-add-segment-int-len-)
   * [int mstrans.set_segment( unsigned segnum, uint8_t* data, unsigned len)](#int-mstrans-set-segment-unsigned-segnum-uint8-t-data-unsigned-len-)
   * [int mstrans.join( uint8_t* info, uint8_t len )](#int-mstrans-join-uint8-t-info-uint8-t-len-)
   * [void mstrans.loop()](#void-mstrans-loop-)


### int mstrans.init( SoftwareSerial& ss, InviteCb invcb, UserCb usercb )
Initializes the communications system and makes necessary preparations.
Must be called before any other function in mstrans.  Returns 0 on success.

###### SoftwareSerial& ss
Reference to a SoftwareSerial initialized to the XBee's data pins.

###### InviteCb invcb
Function to be called when mstrans receives an invitation to join a network.
The user must respond by calling mstrans.join() if he wants to join the network.
The InviteCb type is defined as:

    typedef void (*InviteCb)( uint8_t* info );

Where `info` is a set of information sent by the master to identify itself.

###### UserCb usercb
Function to be called when mstrans receives a USER packet.  The master will
send a USER packet when it needs this callback to handle some user-defined task.
The UserCb type is defined as:

    typedef void (*UserCb)( uint8_t* data );

Where data is some kind of data or instruction sent by the master.
Returns 0 on success.

### int mstrans.add_segment( int len )
Creates a segment of size `len`, when the master requests this segment by index
it will automatically be sent in mstrans.loop() without user action.
Returns 0 on success, 1 if there isn't enough room in the buffer.

### int mstrans.set_segment( unsigned segnum, uint8_t* data, unsigned len)
Fills the segment at index `segnum` with the bytes from `data[0]` to `data[len]`.
Returns 0 on success or 1 if segnum does not exist.

### int mstrans.join( uint8_t* info, uint8_t len )
Sends a JOIN request to the master at address 0, join will not be complete
until master responds ACCEPT.  Returns 0 on success.

### void mstrans.loop()
Main mstrans loop, should be called periodically.

---

## MSTrans Communications Protocol (Hub Perspective)
MSTrans slaves can perform a few communications actions.

#### Join a Network
An mstrans hub can request to join a network with the mstrans.join() function,
though mstrans will net accept requests from the master at address 0 until it
receives an ACCEPT packet.

JOIN:
[1: packet type, 1: checksum, $: variable length info string]

#### Add a Segment
Adding a segment to mstrans creates a data space that will automatically be sent
to the master when requested.  The segment can be any size as long as the length
of all segments together does not exceed BUFFER_SIZE.

#### Update a Segment's data
The mstrans user can update a segment's data with mstrans.set_segment(), this will
update the segment's data that what is provided and the new data will be sent to the
master on any subsequent requests for the segment.

#### Automatically Handled
The mstrans system will automatically handle ACCEPT requests by setting the status
to joined, and will handle GET requests by responding with a SEGMENT packet.

SEGMENT:
[1: packet type, 1: checksum, 1: segment index, $: segment data]

---

## file spi_master.h
This file implements a simple SPI communication protocol.  It works okay, but like
the rest of the code in this directory, it could use a lot of work.

---

## file spi_messaging.h
This file builds on spi_master.h with some convenience function for easily retrieving
slave attributes.

---

## file xbee_init.h
A few convenience functions for XBee initialization.

---

## file mslave.ino
This file implements the main hub program.  It ties the rest of the code together and
implements necessary callbacks.
