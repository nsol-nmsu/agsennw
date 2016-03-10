#include <TimerOne.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include "rtrans.h"
#include "ringbuffer.h"
#include "xbee_init.h"
#include "spi_messaging.h"
#include "spi_master.h"
#include "time.h"

#define SLAVE_TYPE_NONE   0
#define SLAVE_TYPE_HYDRA  1
#define SLAVE_TYPE_ITEMP  2
#define SLAVE_TYPE_OTEMP  3
#define SLAVE_TYPE_HUM    4
#define SLAVE_TYPE_ADC    5
#define SLAVE_TYPE_DUMMY  6

#define DATA_BUFFER_SIZE             100

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
unsigned long sslave_waits[sslave_pin_count] = {0};
char data_buffer[DATA_BUFFER_SIZE];
unsigned data_fill = 0;

unsigned long timeoffset = 0;  //Keep track of time offset relative to global time.h timer


void setup(){
  time_init();
  Serial.begin(9600);
  xs.begin(9600);
  delay(100);
  //xbee_init(xs);
  //rtrans_state.rt_init();
  //Serial.println( "rt initialized");
  Serial.println("started...");
  delay(100);
  message_init(sslave_pins, sslave_pin_count);
  update_slaves();

  Serial.println("ready...");
}

void loop()
{
  //rtrans_state.rt_loop(); 
   data_fill = 0;
   
  //Read all slaves with sslave_wait times of 0
  for( int i = 0 ; i < sslave_count ; i++ )
  {
    if( sslave_waits[i] != 0 )
      continue;

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
    sslave_waits[i] = 3*slave_measure( sslaves[i] );
  }

  //Calculate the time to subtract from sslave_waits and update the timer offset
  unsigned long temp = time_get();
  unsigned long subtime = (temp - timeoffset)*100; //time_get() gives time in deciseconds, convert to miliseconds
  timeoffset = temp;

  //Serial.print("slave_wait = "); Serial.println( sslave_waits[0] );
  //Serial.print("subtime = "); Serial.println( subtime );
  
  //Adjust wait counters
  for( int i = 0 ; i < sslave_count ; i++ )
  {
    if( sslave_waits[i] < subtime )
      sslave_waits[i] = 0;
    else
      sslave_waits[i] -= subtime;
  }

  Serial.write( data_buffer, data_fill );
  
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
  
  rtrans_state.rt_send( RTRANS_TYPE_DATA, (uint8_t*)data_buffer, data_fill );
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
}


