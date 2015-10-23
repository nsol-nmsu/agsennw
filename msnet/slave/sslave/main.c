#include "spi_slave.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "analog.h"
#include "slave.h"

/*
The 'sslave' refered to throughout this file is not declared in this file.
The 'sslave' is a Slave struct from slave.h, and is expected to be defined in an include file.
This allows us to easily switch out slave definitions without changing the main program code.

This file also expects 'void sslave_init()' to be defined elsewhere.  This can be used to initialize
'sslave,' but if initialization is unnecessary it can be an empty function.
*/

//This is a sample sslave include file
#if defined _SLAVE_TTEMP_
#include "ttemp_slave.c"
#elif defined _SLAVE_DUMMY1_
#include "dummy_slave1.c"
#elif defined _SLAVE_DUMMY2_
#include "dummy_slave2.c"
#elif defined _SLAVE_DUMMY3_
#include "dummy_slave3.c"
#else
#include "dummy_slave4.c"
#endif

inline int to_int(char bs[2]);
inline char* from_int(int i);
inline float to_float(char bs[4]);
inline char* from_float(float f);

inline void prep_err();
inline void prep_ok();
void prep_response(char r);
void do_action(char r);

int main(void)
{

  usi_enable();
  spiX_initslave(SPIMODE);
  sei();
  
  //Initialize sslave
  sslave_init();
  
  //Let settings catch up
  _delay_ms(100);
  
  //An initial packet, not sure what it's for, but the other code had it
  prepare_packet("init", 8);
  spiX_put(0);
  
  //Temp
  //analogEnableCustom(PS_64, ADCREF_VCC);
  
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
  float f;
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
      
    //Channel value/output
    case  'q':
      if(arg < sslave.rcount && sslave.rchans[arg] != NULL)
        prepare_packet(from_float(sslave.rchans[arg]()), sizeof(float));
      else
        prep_err();
      //prepare_packet(from_float(getVoltage(ADC_3, 5.0)), sizeof(float));
      break;
      
    //Raw voltage at ADC_X
    case 'v':
      if(arg < sslave.rcount && sslave.rchans[arg] != NULL)
      {
        if(arg >= 8)
          prepare_packet(from_float(getVoltage(ADC_TEMP, sslave.vref)), sizeof(float));
        else
          prepare_packet(from_float(getVoltage(arg, sslave.vref)), sizeof(float));
      }
      else
      {
        prep_err();
      }
      break;
      
    //Unit at X
    case 'u':
      if(arg < sslave.rcount)
        prepare_packet(from_int(sslave.units[arg]), sizeof(int));
      else
        prep_err();
      break;
      
    case 'f':
      prepare_packet(sslave.info, MAX_INFO_LEN);
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
    
    //Change units
    case 'U':
      sslave.units[arg] = to_int(&incoming_packet[3]);
      prep_ok();
      break;
    
    //Change info
    case 'F':
      memcpy(sslave.info, &incoming_packet[2], MAX_INFO_LEN);
      prep_ok();
      break;
      
    //Write to write channel X
    case 'W':
      if(arg < sslave.wcount && sslave.wchans[arg] != NULL)
      {
        sslave.wchans[arg](to_float(&incoming_packet[3]));
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


