#include <SPI.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include "spi_messaging.h"
#include "spi_master.h"


SoftwareSerial xs(6,7);
void update_slaves();

const char sslave_pin_count = 5;
char sslave_pins[] = { 2 , 3 , 4 , 5 , 8 };
char sslave_types[sslave_pin_count]= {0};
char sslave_count = 0;
char sslaves[sslave_pin_count] = {0};


void setup(){
  Serial.begin(9600);
  xs.begin(9600);
  delay(100);
  Serial.println("started...\n");
  message_init(sslave_pins, sslave_pin_count);
  update_slaves();
}

void loop()
{
  int wait = slave_measure( 3 );
  delay(wait);
  Serial.print("wait: "); Serial.println( wait );
  char* val = slave_read( 3, 0 );
  Serial.print("Val: "); Serial.println( val );
  delay(2);
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

