#include "spi_slave.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "analog.h"
#include "slave.h"

//Must have valid slave definition
#include "slave_defs.h"

inline int to_int(char bs[2]);
inline char* from_int(int i);
inline float to_float(char bs[4]);
inline char* from_float(float f);

inline void prep_err();
inline void prep_ok();
void prep_response(char r);
void do_action(char r);

//DEBUG
void set_status( int state )
{
    if( state )
        PORTB |= _BV( 1 );
    else
        PORTB &= ~_BV( 1 );
}

int main(void)
{

    //DEBUG
    DDRB |= _BV(1);

  usi_enable();
  spiX_initslave(SPIMODE);
  sei();
  
  //Initialize sslave
  slave_init();
  
  //Let settings catch up
  _delay_ms(50); //Don't raise this higher than 50, nasty bug when reaches 100
  
  //An initial packet, not sure what it's for, but the other code had it
  prepare_packet("init", 8);
  spiX_put(0);
  
  //Temp
  //analogEnableCustom(PS_64, ADCREF_VCC);
  
  unsigned char input;
  do{
  
  //DEBUG
    //set_status( 0 );
    
    //Wait for SS
    while(!slave_selected());
    
    //DEBUG
         //set_status( 1 );
    
    //Wait for pending transfers
    spiX_wait();
    
    //Read first character from master
    input = spiX_get();
    
    //DEBUG
         //set_status( 1 );
    
    //Is the master telling us to receive?
      //If so, interpret the input and prepare a response packet
    if(input == RECEIVE_CHAR)
    {
    //DEBUG
         //set_status( 1 );
      //Retrieve the rest of the packet from master
      receive_packet();
      
      //Check integrity
      if(!check_integrity())
      {
          prep_err();
          continue;
      }
        
      //Lowercase are read operations
      if(incoming_packet[1] > 96 && incoming_packet[1] < 127)
      {
      //DEBUG
         //set_status( 1 );
        prep_response(incoming_packet[1]);
      }
      else
      {
        do_action(incoming_packet[1]);
      }
    }
    //Is the master telling us that it's ready to receive?
    else if(input == SEND_CHAR)
    {
      //Send the prepared packet
      send_packet();
    }
    
  } while(1);      // Loop forever...
}

union
{
  float f;
  int i;
  char bs[4];
}num_convert;

inline int to_int(char bs[2])
{
  num_convert.bs[0] = bs[0];
  num_convert.bs[1] = bs[1];
  return num_convert.i;
}

inline char* from_int(int i)
{
  static char bs[2];
  num_convert.i = i;
  bs[0] = num_convert.bs[0];
  bs[1] = num_convert.bs[1];
  return bs;
}

inline float to_float(char bs[4])
{
  num_convert.bs[0] = bs[0];
  num_convert.bs[1] = bs[1];
  num_convert.bs[2] = bs[2];
  num_convert.bs[3] = bs[3];
  return num_convert.f;
}

inline char* from_float(float f)
{
  static char bs[4];
  num_convert.f = f;
  bs[0] = num_convert.bs[0];
  bs[1] = num_convert.bs[1];
  bs[2] = num_convert.bs[2];
  bs[3] = num_convert.bs[3];
  return bs;
}

inline void prep_err()
{
  prepare_packet("err", 3);
}

inline void prep_ok()
{
  prepare_packet("ok", 2);
}

void prep_response(char r)
{
  char date[3];
  char arg = incoming_packet[2];
  
  //DEBUG
      //set_status( 1 );
  switch(r)
  {
    //Prepare id to be sent
    case 'i':
      prepare_packet(from_int(sslave.id), sizeof(int));
      break;
      
    //Type
    case 't':
      prepare_packet(from_int(sslave.type), sizeof(int));
      break;
      
    //Name
    case 'n':
      prepare_packet(sslave.name, MAX_NAME_LEN);
      break;
      
    //Deployment Date
    case 'd':
      date[0] = sslave.m;
      date[1] = sslave.d;
      date[2] = sslave.y;
      prepare_packet(date, sizeof(date));
      break;
      
    //Number of read channels
    case 'r':
      prepare_packet(from_int(sslave.rcount),sizeof(int)); 
      break;
      
    //Number of write channels
    case 'w':
      prepare_packet(from_int(sslave.wcount),sizeof(int)); 
      break;
  //Start measurment on channel
    case  'm':
      if(arg < sslave.rcount )
      {
        //DEBUG
        set_status( 1 );
      
        //Wait for measurment
      	unsigned wait = slave_measure( arg );
        prepare_packet( from_int(wait), sizeof( unsigned ) );
      }
      else
        prep_err();
      break;
      
    //Channel value/output
    case  'q':
    //DEBUG
      //set_status( 1 );
      if(arg < sslave.rcount )
      {
      	char* valstr = slave_read( arg );
        prepare_packet( valstr, strlen( valstr ) );
      }
      else
        prep_err();
      break;
      
    case 'f':
      prepare_packet(sslave.info, sslave.ilen);
      break;
    //Echo      
     case 'e':
         prepare_packet("e", 1);
         break;
         
    //Error
    default:
      prep_err();
      break;
  }
}

void do_action(char r)
{
  
  char arg;
  arg = incoming_packet[2];
  
  switch(r)
  {
    //Set the sslave's slave type
    case 'T':
      sslave.type = to_int(&incoming_packet[2]);
      prep_ok();
      break;
      
    //Set the sslave's name
    case 'N':
      memcpy(sslave.name, &incoming_packet[2], MAX_NAME_LEN);
      prep_ok();
      break;
    
    //Change info
    case 'F':
      memcpy(sslave.info, &incoming_packet[2], MAX_INFO_LEN);
      prep_ok();
      break;
      
    //Write to write channel X
    case 'W':
      if(arg < sslave.wcount )
      {
        slave_write( &incoming_packet[3], arg );
        prep_ok();
      }
      else
      {
        prep_err();
      }
      break;
      
    //Error
    default:
      prep_err();
      break;
  }
}


