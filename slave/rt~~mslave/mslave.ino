#include <SPI.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include "rtrans.h"
#include "ringbuffer.h"
#include "xbee_init.h"
#include "spi_messaging.h"
#include "spi_master.h"

#define SLAVE_TYPE_NONE   0
#define SLAVE_TYPE_HYDRA  1
#define SLAVE_TYPE_ITEMP  2
#define SLAVE_TYPE_OTEMP  3
#define SLAVE_TYPE_HUM    4
#define SLAVE_TYPE_ADC    5
#define SLAVE_TYPE_DUMMY  6

#define DATA_BUFFER_SIZE             10

void callback(rt_in_header *header);
void send_poll_response();
SoftwareSerial xs(6,7);
rt_state rtrans_state(xs, callback);
void update_slaves();

const char sslave_pin_count = 5;
char sslave_pins[] = { 2 , 3 , 4 , 5 , 8 };
char sslave_types[sslave_pin_count]= {0};
char sslave_count = 0;
char sslaves[sslave_pin_count] = {0};
char data_buffer[DATA_BUFFER_SIZE];
unsigned data_fill = 0;

unsigned long timeoffset = 0;  //Keep track of time offset relative to global time.h timer


void setup(){
  //time_init();
  Serial.begin(9600);
  xs.begin(9600);
  delay(100);
  Serial.println("started...");
  xbee_init(xs);
  rtrans_state.rt_init();
  Serial.println( "rt initialized");
  Serial.println("started...");
  delay(100);
  message_init(sslave_pins, sslave_pin_count);
  update_slaves();

  Serial.println("ready...");
}

void loop()
{
  rtrans_state.rt_loop();
}

void update_slaves()
{
  for(int i = 0 ; i < sslave_pin_count ; i++)
  {
    if(is_slave(sslave_pins[i]))
    {
      sslaves[sslave_count] = sslave_pins[i];
      Serial.print( "Slave on: " );Serial.println((int)sslave_pins[i]);
      sslave_count++;
    }
  }
}

void send_poll_response()
{
  Serial.println("Sent");
  rtrans_state.rt_send( RTRANS_TYPE_DATA, (uint8_t*)data_buffer, data_fill );
}

void prepare_poll_response()
{
  Serial.println( "Preparing" );
  
  unsigned long sslave_waits[sslave_count];
  data_fill = 0;

  for( int i = 0 ; i < sslave_count ; i++ )
  {
    sslave_waits[i] = slave_measure(sslaves[i]);
  }

  data_buffer[data_fill++] = 'S';
  data_buffer[data_fill++] = 'T';
  data_buffer[data_fill++] = 'A';
  data_buffer[data_fill++] = 'R';
  data_buffer[data_fill++] = 'T';
  data_buffer[data_fill++] = '\r';

  /*unsigned waiting_measures;
  while( waiting_measures )
  {
    waiting_measures = 0;
    
    //Read all slaves
    for( int i = 0 ; i < sslave_count ; i++ )
    {
      if( sslave_waits[i] == 0 )
      {
        data_buffer[data_fill++] = '0' + sslaves[i];
        data_buffer[data_fill++] = ':';
        char* type = get_type( sslaves[i] );
        strcpy( &data_buffer[data_fill], type);
        data_fill += strlen(type);
        data_buffer[data_fill++] = ':';
        int rcount = get_rcount( sslaves[i] );
        data_buffer[data_fill++] = '0' + rcount ;
        data_buffer[data_fill++] = '\n';
        for( int c = 0 ; c < rcount ; c++ )
        {
          char* slave_dat = slave_read( sslaves[i], c );
          strcpy( &data_buffer[data_fill], slave_dat );
          data_fill += strlen( slave_dat );
          data_buffer[data_fill++] = '\n';
        }
        
        data_buffer[data_fill++] = '\r';
      }
      else
      {
        waiting_measures++;
        delay(100);
        for( int i = 0 ; i < sslave_count ; i++ )
        {
          if( sslave_waits[i] >= 100 )
            sslave_waits[i] -= 100;
          else
            sslave_waits[i] = 0;
        }
        
      }
      
    }
    
  }*/

  Serial.println( "Ready");
}

void callback(rt_in_header *header )
{
  Serial.println( "got packet");
  static boolean joined = false;
  if(header->type == RTRANS_TYPE_POLL){
   send_poll_response();
   Serial.println("POLL");
  }
  else if(header->type == RTRANS_TYPE_PROBE && !joined){
    Serial.println("PROBE");
   rtrans_state.rt_join(header->master);
   joined = true;
  }
  else if(header->type == RTRANS_TYPE_PREPARE && joined){
    Serial.println("PREPARE");
   prepare_poll_response();
  }
}


