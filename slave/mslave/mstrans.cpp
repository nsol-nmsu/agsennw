#include "mstrans.h"
#include "xbee_init.h"

using namespace mstrans;

XBee            mstrans::_xbee;
uint16_t        mstrans::_address = 0xFFFF;
//uint16_t        mstrans::_network = MS_FREE_NETWORK;
bool            mstrans::_joined = false; //Temp
UserCb          mstrans::_user_callback;
InviteCb        mstrans::_invite_callback;
uint8_t         mstrans::_buffer[BUFFER_SIZE];
uint8_t*        mstrans::_segments[MAX_SEGMENTS];
unsigned        mstrans::_seg_lengths[MAX_SEGMENTS] = {0};
unsigned        mstrans::_seg_fills[MAX_SEGMENTS] = {0};
uint8_t         mstrans::_segment_count = 0;
void            mstrans::_handle_get_request( uint8_t* data );

uint8_t mstrans::_csum( uint8_t* data, int len )
{
  uint8_t sm = 0;
  for( int i = 0 ; i < len ; i++ )
    sm += data[i];
    
  return 0xFF - sm;
}

int mstrans::init( SoftwareSerial& ss, InviteCb invcb, UserCb usrcb )
{
        _invite_callback = invcb;
        _user_callback   = usrcb;
        
        mstrans::_xbee.setSerial( ss );
        
        /*Set initial XBee configuration*/
        xbee_init( ss );
        
        /*Configure  the XBee address*/
        //Request low serial bytes
        AtCommandResponse resp;
        AtCommandRequest sl_at( (uint8_t*)"SL" );
        int retries = 5;
        while( retries-- )
        {
                _xbee.send( sl_at );
                
                //Wait for response
                _xbee.readPacket( 100 );
                if( _xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )
                {
                        _xbee.getResponse().getAtCommandResponse( resp );
                        if( resp.isOk() )
                                break;
                        else
                                continue;
                }
        }
        
        //If we ran out of retries there was a problem
        if( retries == 0 )
        {
                DEBUG("AT Response Timeout\n" );
                return 1;
        }
        
       // _address = resp.getValue()[1] | (resp.getValue()[0] << 8 );
        
        //Request address change
        retries = 5;
        AtCommandRequest my_at( (uint8_t*)"MY", resp.getValue(), 2 );
        while( retries-- )
        {
                _xbee.send( sl_at );
                
                //Wait for response
                _xbee.readPacket( 100 );
                if( _xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )
                {
                        _xbee.getResponse().getAtCommandResponse( resp );
                        if( resp.isOk() )
                                break;
                        else
                                continue;
                }
        }
        
        //If we ran out of retries there was a problem
        if( retries == 0 )
        {
                DEBUG("AT Response Timeout\n" );
                return 1;
        }
        
        return 0;
}

int mstrans::add_segment( int len )
{
        //Figure out where to start the segment
        int seg_start = 0;
        for( int i = 0 ; i < _segment_count ; i++ )
                seg_start += _seg_lengths[i];
        
        //Figure out how much room we have left
        len = ( BUFFER_SIZE - seg_start >= len )? len : BUFFER_SIZE - seg_start;
        if( len <= 0 )
                return 0;
        
        //Setup the segment
        _segments[_segment_count] = &_buffer[seg_start];
        _seg_lengths[_segment_count] = len;
        _segment_count++;
        
        return len;
}

int mstrans::set_segment( unsigned segnum, uint8_t* data, unsigned len )
{
        if( segnum >= _segment_count )
                return 1;

        int i = 0;
        for( ; i < _seg_lengths[segnum] && i < len ; i++ )
        {
                _segments[segnum][i] = data[i];
        }

        _seg_fills[segnum] = i;
        return 0;
}

int mstrans::join( uint8_t* info, uint8_t len )
{       
        int buffsize = 2 + len;
        uint8_t buffer[ buffsize ];
        buffer[0] = MSPACKET_JOIN;
        buffer[1] = 0;                          //Replace with checksum later
        memcpy( &buffer[2], info, len );
        buffer[1] = _csum( buffer, buffsize );
        Tx16Request join_request( 0x00, buffer, buffsize );
        _xbee.send( join_request );
        return 0;
        
}

/*
  int mstrans::join( uint16_t network, uint8_t* info, uint8_t len )
{
        int buffsize = 2 + sizeof(network) + len;
        uint8_t buffer[ buffsize ];
        buffer[0] = MSPACKET_JOIN;
        buffer[1] = 0;                          //Replace with checksum later
        memcpy( &buffer[2], &network, 2 );
        memcpy( &buffer[3], info, len );
        buffer[1] = _csum( buffer, buffsize );
        Tx16Request join_request( 0, buffer, buffsize );
        
        //Send join request and wait for confirmation
        int retries = 10;
        while( retries-- )
        {
                _xbee.send( join_request );
                
                //Wait for response
                Rx16Response resp;
                _xbee.readPacket( 200 );
                if( _xbee.getResponse().getApiId() == RX_16_RESPONSE )
                {
                        _xbee.getResponse().getRx16Response( resp );
                        if( resp.getData()[0] == MSPACKET_OK )
                        {
                                _network = network;
                                break;
                        }
                }
        }
        
        //If we ran out of retries then a timeout occured
        if( retries == 0 )
        {
                DEBUG("Join Response Timeout\n" );
                return 1;
        }
        
        //Now change the XBee's network
        retries = 5;
        uint16_t net64[4] = {0};
        net64[3] = _network;
        AtCommandRequest id_at( (uint8_t*)"ID", (uint8_t*)&net64, 8 );
        while( retries-- )
        {
                _xbee.send( id_at );
                
                //Wait for response
                AtCommandResponse resp;
                _xbee.readPacket( 100 );
                if( _xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )
                {
                        _xbee.getResponse().getAtCommandResponse( resp );
                        if( resp.isOk() )
                                break;
                }
        }
        
        //If we ran out of retries there was a problem
        if( retries == 0 )
        {
                DEBUG("AT Response Timeout\n" );
                return 1;
        }
        
        return 0;
        
}
 */

/*int mstrans::leave()
{
        //Change network to MS_FREE_NETWORK
        _network = MS_FREE_NETWORK;
        int retries = 5;
        uint16_t net64[4] = {0};
        net64[3] = _network;
        AtCommandRequest id_at( (uint8_t*)"ID", (uint8_t*)&net64, 8 );
        while( retries-- )
        {
                _xbee.send( id_at );
                
                //Wait for response
                AtCommandResponse resp;
                _xbee.readPacket( 100 );
                if( _xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE )
                {
                        _xbee.getResponse().getAtCommandResponse( resp );
                        if( resp.isOk() )
                                break;
                }
        }
        
        //If we ran out of retries there was a problem
        if( retries == 0 )
        {
                DEBUG("AT Response Timeout\n" );
                return 1;
        }
        
        return 0;
        
}*/

void mstrans::loop()
{
        if( _xbee.readPacket(100) )
        {
                DEBUG("Received\n");
                
                if( _xbee.getResponse().getApiId() == RX_16_RESPONSE )
                {
                        Rx16Response resp;
                        _xbee.getResponse().getRx16Response( resp );
                        uint8_t* data = resp.getData();
                        uint8_t  datalen = resp.getDataLength();
                        
                        //Verify checksum
                        int check = 0;
                        for( int i = 0 ; i < datalen ; i++ )
                                check += data[i];
                        if( check != 0xFF )
                        {
                                DEBUG("Bad checksum ");DEBUG( check );
                                return;
                        }
                        
                        uint8_t resp_type = data[0];
                        
                        switch( resp_type )
                        {
                                case MSPACKET_USER:
                                        if(_joined)
                                          _user_callback( &data[2] );
                                        break;
                                case MSPACKET_GET:
                                        if(_joined)
                                          _handle_get_request( &data[2] );
                                        break;
                                case MSPACKET_INVITE:
                                        if( !_joined )
                                          _invite_callback( &data[2] );
                                        break;
                                case MSPACKET_ACCEPT:
                                        _joined = true;
                                        break;
                                default:
                                        DEBUG("Unexpected packet\n");
                                        break;
                        }
                }
                else if ( _xbee.getResponse().isError() )
                {
                        DEBUG("Communications Error\n");
                        //TODO: Handle errors
                }
        }
}

void mstrans::_handle_get_request( uint8_t* data )
{
        uint8_t request_segs = data[0]; //Number of segments requested; 0 = ALL
        uint8_t seg_nums[ ( request_segs )? request_segs : _segment_count - 1 ];

        if( request_segs == 0 && _segment_count > 0 )
        {
                request_segs = _segment_count;
                for( int i = 0 ; i < request_segs ; i++ )
                        seg_nums[i] = i;
        }
        else
        {
                for( int i = 0 ; i < request_segs ; i++ )
                        seg_nums[i] = data[ i + 1 ];
        }
        
        for( int i = 0 ; i < request_segs ; i++ )
        {
                //Generate segment packets
                uint8_t  seg_len  = _seg_fills[ seg_nums[i] ];
                uint8_t  seg_data[ seg_len + 3 ];
                seg_data[0] = MSPACKET_SEGMENT;
                seg_data[1] = 0;
                seg_data[2] = seg_nums[ i ];
                memcpy( &seg_data[3], _segments[ seg_nums[i] ], seg_len );
                seg_data[1] = _csum( seg_data, seg_len + 3 );
                
                Tx16Request tx_request( 0, seg_data, seg_len + 3 );
                _xbee.send( tx_request );
                DEBUG("Sent Segment "); DEBUG( seg_nums[i] );
                DEBUG(" of "); DEBUG(seg_len + 3);DEBUG("\n");
        }
        
}

