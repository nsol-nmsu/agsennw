#include "slave.h"
#include <stdlib.h>
#include "analog.h"


/**
A simple hub for external ADC temperature sensor and humidity sensor.
Read Channels: 1
Write Channels: 0
**/
#include "analog.h"

int slave_init()
{
  analogEnableDefault();
  return 0;
};

unsigned slave_measure()
{
        return 0;
};

void slave_run_measure()
{
        return;
};


char* slave_read( unsigned ch )
{
  double temp_val = 50.0 + (getVoltage(ADC_2, 5.0) - 2.5) * 100.0;
  double hum_val = ( getVoltage(ADC_3, 5.0)/5.0 - 0.16 ) / 0.0062;
  static char str[40];
  int i = 0;
  str[i++] = 't'; str[i++] = 'e'; str[i++] = 'm'; str[i++] = 'p';
  str[i++] = ':';
  dtostrf( temp_val, 5, 3, &str[i] );
  i += 5;
  str[i++] = ':';
  str[i++] = 'F';
  str[i++] = '\n';
  
  str[i++] = 'h'; str[i++] = 'u'; str[i++] = 'm';
  str[i++] = ':';
  dtostrf( hum_val, 5, 3, &str[i] );
  i += 5;
  str[i++] = ':';
  str[i++] = 'R';
  str[i++] = 'H';
  str[i++] = '\n';
  str[i++] = 0;
  return str;
};

int slave_write( const char* msg, unsigned ch )
{
        return 1;
};

int slave_apply( unsigned ch )
{
        return 1;
};

int   slave_id = 103;
char* slave_type = "hum-temp";
char* slave_name = "humidity and temperature";
char  slave_init_date[3] = { 27, 2, 16 };
char  slave_rcount = 1;
char  slave_wcount = 0;
char* slave_info = "A simple ADC humidity and temperature duality";
