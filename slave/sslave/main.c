#include "spi_slave.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "slave.h"

char waiting_measure = 0;

inline int to_int(char bs[2]);
inline char* from_int(int i);
inline float to_float(char bs[4]);
inline char* from_float(float f);

inline void prep_err();
inline void prep_ok();
void prep_response(char r);
void do_action(char r);

char buffer[MAX_PACKET_LENGTH];

int main(void)
{
    
  usi_enable();
  spiX_initslave(SPIMODE);
  sei();
  

  //Initialize slave
  slave_init();
  
  //Let settings catch up
  _delay_ms(100);
  
  //An initial packet, not sure what it's for, but the other code had it
  prepare_packet("", 0);
  spiX_put(0);
  
  unsigned char input;
  do{
    
    //Wait for SS
    while(!slave_selected());
    
    //Wait for pending transfers
    spiX_wait();
    
    //Read first character from master
    input = spiX_get();
    
    //Is the master telling us to receive?
      //If so, interpret the input and prepare a response packet
    if(input == RECEIVE_CHAR)
    {
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
    if(waiting_measure){
       slave_run_measure();
       waiting_measure = 0;
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
  char arg = incoming_packet[2];
  
  switch(r)
  {
    //Prepare id to be sent
    case 'i':
      prepare_packet(from_int(slave_id), sizeof(int));
      break;
      
    //Type
    case 't':
      prepare_packet( slave_type, strlen( slave_type ) );
      break;
      
    //Name
    case 'n':
      prepare_packet(slave_name, strlen( slave_name ) );
      break;
      
    //Deployment Date
    case 'd':
      prepare_packet(slave_init_date, sizeof(slave_init_date));
      break;
      
    //Number of read channels
    case 'r':
      prepare_packet(from_int(slave_rcount),sizeof(int)); 
      break;
      
    //Number of write channels
    case 'w':
      prepare_packet(from_int(slave_wcount),sizeof(int)); 
      break;
  //Start measurment on channel
    case  'm':
    {
      //Wait for measurment
      unsigned wait = slave_measure();
      prepare_packet( from_int(wait), sizeof( unsigned ) );
      waiting_measure = 1;
    }
      break;
      
    //Channel value/output
    case  'q':
      if(arg < slave_rcount )
      {
      	char* valstr = slave_read( arg );
        prepare_packet( valstr, strlen( valstr ) );
      }
      else
        prep_err();
      break;
      
    case 'f':
      prepare_packet(slave_info, strlen( slave_info ) );
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
    //Set the slave type
    case 'T':
      memcpy(slave_type, &incoming_packet[3], arg);
      prep_ok();
      break;
      
    //Set the sslave's name
    case 'N':
      memcpy(slave_name, &incoming_packet[3], arg);
      prep_ok();
      break;
    
    //Change info
    case 'F':
      memcpy(slave_info, &incoming_packet[3], arg);
      prep_ok();
      break;
      
    //Write to write channel X
    case 'W':
      if(arg < slave_wcount )
      {
        slave_write( (char*)&incoming_packet[3], arg );
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


