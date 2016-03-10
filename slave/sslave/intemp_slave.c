#include "slave.h"
#include <stdlib.h>
#include "analog.h"

/**
A simple test sensor that utilizes the AVR's internal temperature sensor.
Read Channels: 1
Write Channesl: 0
**/

int slave_init()
{
  analogEnableCustom(PS_64, ADCREF_INT );
  return 0;
}

unsigned slave_measure( unsigned ch )
{
        return 0;
};


char* slave_read( unsigned ch )
{
  double val = analogGet(ADC_TEMP) - 270;
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

Slave sslave =
{
  .id = 102,
  .type = 2,
  .name = "itemp",
  .m = 10, .d = 19, .y = 15,
  .rcount = 1,
  .wcount = 0,
  .ilen = 25,
  .info = "Internal AVR termperature"
};
