#include <SPI.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include "rtrans.h"
#include "ringbuffer.h"
#include "xbee_init.h"
#include "spi_messaging.h"
#include "spi_master.h"


void callback(rt_in_header *header);
void update_slaves();
void send_poll_response();
SoftwareSerial xs(6,7);
rt_state rtrans_state(xs, callback);

const char sslave_pin_count = 7;
char sslave_pins[] = { 2 , 3 , 4 , 5 , 8 , 9 , 10};
char sslave_count = 0;
char sslaves[sslave_pin_count] = {0};

void setup(){
  Serial.begin(9600);
 xs.begin(9600);
  delay(100);
  xbee_init(xs);
  rtrans_state.rt_init();
  Serial.println( "rt initialized");
  Serial.println("started...");
  delay(100);
  message_init(sslave_pins, sslave_pin_count);
  update_slaves();
}

void loop()
{
  rtrans_state.rt_loop(); 
  //Serial.println(read_chan(9, 0));
}

void update_slaves()
{
  for(int i = 0 ; i < sslave_pin_count ; i++)
  {
    if(is_slave(sslave_pins[i]))
    {
      sslaves[sslave_count] = sslave_pins[i];
      sslave_count++;
      Serial.print( "Slave on: " );Serial.println((int)sslave_pins[i]);
    }
  }
}

typedef struct
{
  char name[20];
  int  id;
  int type;
  char value[10];
}SlaveData;

void send_poll_response()
{
  
  SlaveData data[sslave_count];

  for(int i = 0 ; i < sslave_count ; i++)
  {
    strcpy( data[i].name, get_name(sslaves[i]) );
    data[i].id = get_id(sslaves[i]);
    data[i].type = get_type( sslaves[i] );
    
    while( measure_chan( sslaves[i], 0 ) > 0 ) delay( 100 );
    
    strcpy( data[i].value, read_chan(sslaves[i], 0) );
    
  }

  rtrans_state.rt_send(RTRANS_TYPE_DATA, (unsigned char*)data, sizeof(data));
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
