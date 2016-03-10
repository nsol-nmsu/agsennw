#include "slave.h"
#include <stdlib.h>
#include "analog.h"


/**
A simple sensor using an external analog temperature sensor connected to ADC_3
Read Channels: 1
Write Channels: 0
**/
#include "analog.h"

int slave_init()
{
  analogEnableDefault();
  return 0;
}

unsigned slave_measure()
{
        return 0;
};


char* slave_read( unsigned ch )
{
  double val = 50.0 + (getVoltage(ADC_3, 5.0) - 2.5) * 100.0;
  static char str[20];
  str[0] = 't'; str[1] = 'e'; str[2] = 'm'; str[3] = 'p';
  str[4] = ':';
  dtostrf( val, 5, 3, &str[5] );
  str[10] = ':';
  str[11] = 'F';
  str[12] = '\n';
  str[13] = 0;
  return str;
};

int slave_write( const char* msg, unsigned ch )
{
        return 1;
}

int slave_apply( unsigned ch )
{
        return 1;
};

int   slave_id = 103;
char* slave_type = "ext-temp";
char* slave_name = "external tempurature";
char  slave_init_date[3] = { 27, 2, 16 };
char  slave_rcount = 1;
char  slave_wcount = 0;
char* slave_info = "A simple ADC tempurature sensor.  Sensor connects to ADC3 of Attiny84";
