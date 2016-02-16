#ifndef _SLAVE_DEFS_H_
#define _SLAVE_DEFS_H_

#include "slave.h"
#include <stdlib.h>

//#define INTERNAL_TEMP //Sensor type to use
//#define EXTERNAL_TEMP
//#define HYDRA_PROBE
#define DUMMY

#ifdef INTERNAL_TEMP
/**
A simple test sensor that utilizes the AVR's internal temperature sensor.
Read Channels: 1
Write Channesl: 0
**/
#include "analog.h"

void slave_init()
{
  analogEnableCustom(PS_64, ADCREF_INT );
}

unsigned slave_measure( unsigned ch )
{
        return 0;
};

char* slave_read( unsigned ch )
{
  double val = analogGet(ADC_TEMP) - 270;
  static char str[10];
  dtostrf( val, 10, 5, str );
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
  .type = 1,
  .name = "itemp",
  .m = 10, .d = 19, .y = 15,
  .rcount = 1,
  .wcount = 0,
  .ilen = 25,
  .info = "Internal AVR termperature"
};
#elif defined EXTERNAL_TEMP
/**
A simple sensor using an external analog temperature sensor connected to ADC_3
Read Channels: 1
Write Channesl: 0
**/
#include "analog.h"

void slave_init()
{
  analogEnableDefault();
}

unsigned slave_measure( unsigned ch )
{
        return 0;
};

char* slave_read( unsigned ch )
{
  double val = 50.0 + (getVoltage(ADC_3, 5.0) - 2.5) * 100.0;
  static char str[10];
  dtostrf( val, 10, 5, str );
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
  .id = 103,
  .type = 2,
  .name = "itemp",
  .m = 10, .d = 19, .y = 15,
  .rcount = 1,
  .wcount = 0,
  .ilen = 25,
  .info = "External AVR termperature"
};

#elif defined HYDRA_PROBE
/**
An SDI12 sensor for sensing soil moisture and temperature
**/
#include "sdi.h"

static uint8_t response_buffer[ MAX_READ ];

void slave_init()
{
        sdi_init(&DDRB, &PORTB, &PINB, 0);
}

unsigned slave_measure( unsigned ch )
{
        sdi_exchange("1M2!", response_buffer );
        
        //We don't need last item in response, so 
        //change it to null to terminate the string
        response_buffer[4] = 0;
        int wait = atoi( &response_buffer[1] );
        return wait * 1000;
};

char* slave_read( unsigned ch )
{
  static char value[ MAX_PACKET_LENGTH ];
  
  sdi_exchange("1D0!", response_buffer );
  int section = 0;
  int start = 0;
  while( section <= ch )
  {
        if( response_buffer[start] == '+' || response_buffer[start] == '-' )
                section++;
        start++;
  }
  
  int end = start+1;
  while( response_buffer[end] != '+' && response_buffer[end] != '-' 
        && response_buffer[end] != 0 )
        end++; 
  
  memcpy( value, &response_buffer[start], end - start );
  return value;
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
  .id = 104,
  .type = 3,
  .name = "itemp",
  .m = 10, .d = 19, .y = 15,
  .rcount = 1,
  .wcount = 0,
  .ilen = 29,
  .info = "HydraProbe SDI-12 Soil Sensor"
};

#elif defined DUMMY
/**
A dummy slave.  Increments it's output every request.
**/

void slave_init()
{
}

unsigned slave_measure( unsigned ch )
{
        return 0;
};

char* slave_read( unsigned ch )
{
  static char value[ MAX_PACKET_LENGTH ];
  static unsigned dummy = 0;
  itoa( dummy, value, MAX_PACKET_LENGTH );
  return value;
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
  .id = 104,
  .type = 3,
  .name = "dummy",
  .m = 10, .d = 19, .y = 15,
  .rcount = 1,
  .wcount = 0,
  .ilen = 29,
  .info = "A simple dummy slave."
};

#endif //DUMMY


#endif //_SLAVE_DEFS_H_
