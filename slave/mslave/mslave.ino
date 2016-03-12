#include <SPI.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include "mstrans.h"
#include "spi_messaging.h"
#include "spi_master.h"

#define DATA_BUFFER_SIZE             500
#define SEG_SIZE                     60


SoftwareSerial xs(6,7);
void update_slaves();

const char sslave_pin_count = 5;
char sslave_pins[] = { 2 , 3 , 4 , 5 , 8 };
char sslave_types[sslave_pin_count]= {0};
char sslave_count = 0;
char sslaves[sslave_pin_count] = {0};
char data_buffer[DATA_BUFFER_SIZE];
unsigned data_fill = 0;

unsigned long timeoffset = 0;  //Keep track of time offset relative to global time.h timer

void invite_cb( uint8_t* info )
{
  mstrans::join((uint8_t*)&sslave_count, 1 );
  Serial.println( "joined" );
}

void user_cb( uint8_t* data )
{
  Serial.println("Set Segments");
  unsigned slave_waits[sslave_count];
  for( int i = 0; i < sslave_count ; i++ )
  {
    slave_waits[i] = slave_measure( sslaves[i] );
    DEBUG("Wait is: "); DEBUG( slave_waits[i] ); DEBUG('\n');
  }

  unsigned waiting;
  do
  {
    waiting = 0;
    for( int i = 0 ; i < sslave_count ; i++ )
    {
      if( slave_waits[i] == 0 )
      {
          uint8_t data[SEG_SIZE];
          unsigned idx = 0;
          uint8_t* type = (uint8_t*)get_type( sslaves[i] );

          sprintf( (char*)data, "%u:%s\n", sslaves[i], type );
          idx += strlen( (char*)data );

          unsigned rcount = get_rcount( sslaves[i] );
          for( int j = 0 ; j < rcount ; j++ )
          {
            if( idx >= SEG_SIZE )
              break;

            uint8_t* val = (uint8_t*)slave_read( sslaves[i], j );
            strncpy( (char*)&data[idx], (char*)val, incoming_packet[0] - 5 );
            idx += strlen( (char*)val );
          }
          mstrans::set_segment( i, data, idx );
      }
      else
      {
        waiting++;
        if( slave_waits[i] < 100 )
          slave_waits[i] = 0;
        else
          slave_waits[i] -= 100;
        delay(100);
      }
    }
    
  }while( waiting );
}

void setup(){
  Serial.begin(9600);
  xs.begin(9600);
  delay(100);
  Serial.println("started...\n");
  message_init(sslave_pins, sslave_pin_count);
  update_slaves();

  if( !mstrans::init( xs, invite_cb, user_cb ) )
    Serial.println("mstrans initialized");
  else
    Serial.println("mstrans failed to initialize");
  Serial.println("ready...");

  //Setup segments
  for( int i = 0 ; i < sslave_count ; i++ )
    mstrans::add_segment( SEG_SIZE );
}

void loop()
{
  //Serial.println('.');
  mstrans::loop();
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

