/*
A simple SDI-12 library for AVR microcontrollers.

This module uses the util/delay.h module and so requires optimization to be enabled
for accurate delays.
*/

#ifndef _SDI_H_
#define _SDI_H_



#include <stdint.h>
#include <util/delay_basic.h>
#include <util/parity.h>
#include <avr/io.h>

#ifndef F_CPU
#error sdi12.h requires a definition for F_CPU, none found.
#endif

//Timing for tiny was difficult, so calibrated manually: Expects attiny84 at 8Mhz
#define SDI_SPACE_DELAY         _delay_loop_2(830*2 + 15);
#define SDI_HALF_SPACE_DELAY    _delay_loop_2(830 + 5);
#define SDI_BREAK_DELAY         _delay_loop_2(12000*2 + 200);
#define SDI_POST_BREAK_DELAY    _delay_loop_2(8300*2 + 200);
#define SDI_100MS_DELAY         _delay_loop_2(0);\
                                        _delay_loop_2(0);
//Other
#define MAX_READ                76
#define TIMEOUT_CYCLE_COUNT     30000

//Use macros for values since ArduinoStudio screws up C enums
typedef uint8_t LineDirection;
#ifndef INPUT
#       define INPUT   0
#endif
#ifndef OUTPUT
#       define OUTPUT  1
#endif

typedef uint8_t Bool;
#define FALSE   0
#define TRUE    1

/*
Port and pin settings
*/
static volatile uint8_t* sdi_dport; //Direction port
static volatile uint8_t* sdi_oport;  //Out port
static volatile uint8_t*  sdi_iport;  //In port
static volatile uint8_t  sdi_pin;   //Pin

void sdi_set( Bool state );
void sdi_set_direction( LineDirection dir );

/*
Initialize the module.
uint8_t*        dport    = Port address of data line direction register
uint8_t*        port     = Port address for data line
uint8_t         pin      = Pin # for data line
*/
void sdi_init( volatile uint8_t* dport, volatile uint8_t* oport, volatile uint8_t* iport , uint8_t pin )
{
        sdi_dport = dport;
        sdi_oport  = oport;
        sdi_iport  = iport;
        sdi_pin   = pin;
        sdi_set_direction( OUTPUT );

        //Set the data line to marking for 100 ms to clear all garbage and noise
        sdi_set( FALSE );
        SDI_100MS_DELAY
        
}

/*
Set the data line direction: input/output
*/
void sdi_set_direction( LineDirection dir )
{
         switch( dir )
         {
                case INPUT:
                        *sdi_dport &= ~_BV( sdi_pin );
                        return;
                case OUTPUT:
                        *sdi_dport |= _BV( sdi_pin );
                        return;
                        
         }
}

/*
Set the data line's state
*/
void sdi_set( Bool state )
{
        if( state )
                *sdi_oport |= _BV( sdi_pin );
        else
                *sdi_oport &= ~_BV( sdi_pin );
}

/*
Read the data line's state
*/
Bool sdi_get()
{
        if ( *sdi_iport & _BV( sdi_pin ) ) return TRUE;
        return FALSE;
}

/*
Sets the data line to marking ( 0 - 1.0 volts )
# SDI12 uses negative logic.
*/
void sdi_mark()
{
        //Clear the bit since SDI12 uses reverse logic
        sdi_set( FALSE );
        SDI_SPACE_DELAY
}

/*
Sets the data line to flat/space ( 3.5 - 5.5 volts )
*/
void sdi_space()
{
        //Set the bit
        sdi_set( TRUE );
        SDI_SPACE_DELAY
}

/*
Wait for the data line to be marking
*/
void sdi_wait_mark()
{
        //Wait while pin is HIGH
        while( sdi_get() );
        return;
}

/*
Wait for the data line to be unmarked.
*/
void sdi_wait_space()
{
        //Wait while pin is low
        while( !sdi_get() );
        return;
}

/*
Read a char
*/
uint8_t sdi_read()
{
        uint8_t ch = 0;

        //15ms is max allowed timing between request and response: wait for the start bit
        unsigned long count = TIMEOUT_CYCLE_COUNT;
        while( !sdi_get() && count-- )
                _delay_loop_2(1);
                
        //If count == 0, response didn't come, return 127
        if( !count )
                return 127;
                
        
        //Wait for 1/2 bit spacing to set sample in the middle of start bit
        SDI_HALF_SPACE_DELAY
        
        //Read all data bits
        uint8_t mask = 1;
        for( ; mask < 0x80 ; mask <<= 1 )
        {
                //Wait for the middle of the next bit
                SDI_SPACE_DELAY
                
                if( !sdi_get() )
                        ch |= mask;
                        
        }
        
        //Skip the stop and parity bits
        SDI_SPACE_DELAY
        SDI_SPACE_DELAY

        //Otherwise return ch
        return ch;
}

/*
Write a char
*/
void sdi_write( uint8_t ch )
{
        //Write the start bit
        sdi_set( TRUE ); //Start bit
        SDI_SPACE_DELAY
        
        uint8_t mask = 1;
        for( ; mask & 0b01111111 ; mask <<= 1 )
        {
                if( ch & mask )
                        sdi_mark();
                else
                        sdi_space();
        }
        
        if( parity_even_bit( ch ) )
                sdi_mark();
        else
                sdi_space();
        
        //Write stop bit
        sdi_set( FALSE );
        SDI_SPACE_DELAY
        
        return;
}

/*
Send a command and retrieve the response. 'command' must be null terminated.
*/
Bool sdi_exchange( uint8_t* command, uint8_t* response )
{
          
        //Generate break ( 12ms of space )
        sdi_set_direction( OUTPUT );
        sdi_set( TRUE );
        SDI_BREAK_DELAY
        sdi_set( FALSE );
        SDI_POST_BREAK_DELAY

        //Write all data
        uint8_t* c;
        for( c = command ; *c != 0 ; c++ )
        {
                sdi_write( *c );
        }
        
        
        sdi_set_direction( INPUT );
        
        //Read all input until <CR><LF>
        int idx = 0;
        while( !( idx > 1 && response[ idx - 2 ] == '\r' && response[ idx - 1 ] == '\n' )
                && idx < MAX_READ - 1 )
        {
                response[ idx ] = sdi_read();
                if( response[ idx ] == 127 )
                  return FALSE;
                  
                idx++;
        }
        response[MAX_READ - 1] = 0; //Null terminate
        
        return TRUE;
}

#endif //_SDI_H_
