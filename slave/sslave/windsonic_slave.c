/**
An SDI12 HydraProbe II sensor for sensing soil moisture and temperature
**/
#include "sdi.h"
#include "slave.h"
#include <stdlib.h>
#include <util/delay_basic.h>

uint8_t response_buffer[MAX_READ];

int slave_init()
{
        sdi_init(&DDRB, &PORTB, &PINB, 0);
        return 0;
};

unsigned slave_measure( unsigned ch )
{
        sdi_exchange((uint8_t*)"0M0!", response_buffer );
        
        //We don't need last item in response, so 
        //change it to null to terminate the string
        response_buffer[4] = 0;
        int wait = atoi( (char*)&response_buffer[1] );
        
        return wait * 1000;
        //return 2;
};

void slave_run_measure()
{
        return;
};

static int next_val( uint8_t* str)
{
        int i = 0;
        do{ i++; }while( str[i] != '-' 
                        && str[i] != '+' 
                        && str[i] != '\r'
                        && str[i] != '\n' 
                        && str[i] != '\0' );
        return i;
};

char* slave_read( unsigned ch )
{
  if(!sdi_exchange((uint8_t*)"0D0!", response_buffer ))
  {
        response_buffer[0] = 'e';
        response_buffer[0] = 'r';
        response_buffer[0] = 'r';
        response_buffer[0] = '\r';
        response_buffer[0] = '\n';
  }
  
  return response_buffer;
};

int slave_write( const char* msg, unsigned ch )
{
        return 1;
};

int slave_apply()
{
        return 1;
};

int   slave_id = 104;
char* slave_type = "windsonic";
char* slave_name = "windsonic bridge";
char  slave_init_date[3] = { 27, 2, 16 };
char  slave_rcount = 1;
char  slave_wcount = 0;
char* slave_info = "A bridge between the sensor hub and a Gill Winsonic SDI-12 \
sensor.  Probe connects to PORTB:0";
