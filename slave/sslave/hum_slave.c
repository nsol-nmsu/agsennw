#include "slave.h"
#include <stdlib.h>
#include "analog.h"


/**
Pretty much a copy of the adc_slave.c file, only change is the .id field in sslave
Read Channels: 1
Write Channesl: 0
**/
#include "analog.h"

int slave_init()
{
  analogEnableDefault();
  return 0;
}

unsigned slave_measure( unsigned ch )
{
        return 0;
};


char* slave_read( unsigned ch )
{
  double val = getVoltage(ADC_3, 5.0);
  static char str[20];
  int i = 0;
  str[i++] = 'h'; str[i++] = 'u'; str[i++] = 'm';
  str[i++] = ':';
  dtostrf( val, 5, 3, &str[i] );
  i += 5;
  str[i++] = ':';
  str[i++] = 'r';
  str[i++] = 'a';
  str[i++] = 'w';
  str[i++] = '\n';
  str[i++] = 0;
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

int   slave_id = 105;
char* slave_type = "humid";
char* slave_name = "humidity";
char  slave_init_date[3] = { 27, 2, 16 };
char  slave_rcount = 1;
char  slave_wcount = 0;
char* slave_info = "ADC humidity sensor, returns raw voltage.  No transforms.";
