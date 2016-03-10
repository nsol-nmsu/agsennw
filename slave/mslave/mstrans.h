#ifndef _MS_TRANS_H_
#define _MS_TRANS_H_

#include <XBee.h>
#include <SoftwareSerial.h>

#define DEBUG( s ) Serial.print( s )

#define MAX_SEGMENTS    10
#define MS_AT_TIMEOUT   100
#define MS_AT_RETIRES   5
#define BUFFER_SIZE     500

#define MSPACKET_JOIN           0       //Slave to master, request to join network
#define MSPACKET_SEGMENT        1       //Slave to master, segment packet
#define MSPACKET_USER           2       //Master to slave, trigger callback
#define MSPACKET_GET            3       //Master to slave, request segments
#define MSPACKET_OK             4       //General confirmation packet
#define MSPACKET_INVITE         5       //Broadcast by master
#define MSPACKET_ACCEPT         6       //Master to slave, accept join request

#define MS_FREE_NETWORK         1       //Starting network of all slaves

typedef void (*UserCb)( uint8_t* data );
typedef void (*InviteCb)( uint8_t* info );

namespace mstrans
{  
        extern XBee            _xbee;
        extern uint16_t        _address;
        //extern uint16_t        _network;
        extern bool            _joined; //Temp
        extern UserCb          _user_callback;
        extern InviteCb        _invite_callback;
        extern uint8_t         _buffer[BUFFER_SIZE];
        extern uint8_t*        _segments[MAX_SEGMENTS];
        extern unsigned        _seg_lengths[MAX_SEGMENTS];
        extern unsigned        _seg_fills[MAX_SEGMENTS];
        extern uint8_t         _segment_count;
        extern void            _handle_get_request( uint8_t* data );
        extern uint8_t         _csum( uint8_t* data, int len );
        
        extern int             init( SoftwareSerial& ss, InviteCb invcb, UserCb usrcb );
        extern int             add_segment( int len );
        extern int             set_segment( unsigned segnum, uint8_t* data, unsigned len );
        extern int             join( uint8_t* info, uint8_t len );
        //extern int             join( uint16_t network, uint8_t* info, uint8_t len );
        //extern int             leave();
        extern void            loop();
}
#endif /* _MS_TRANS_H_ */
