/*
A simple SDI-12 library for AVR microcontrollers.

This module uses the util/delay.h module and so requires optimization to be enabled
for accurate delays.
*/

#ifndef _SDI_H_
#define _SDI_H_



#include <stdint.h>
#include <util/delay.h>
#include <util/parity.h>
#include <avr/io.h>

#ifndef F_CPU
#error sdi12.h requires a definition for F_CPU, none found.
#endif

//Timing is in micro seconds
#define MAX_RESPONSE_DELAY      15000
#define BREAK_DELAY             12100
#define POST_BREAK_DELAY        8400
#define SPACING                 830  // Microseconds
#define MAX_READ                76
#define CYCLES_FOR_15MS         ( F_CPU * 15UL / 1000UL )
#define CYCLES_PER_BIT_WAIT     10 //Just a guess, not tested nor calculated

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

inline void sdi_set( Bool state );
inline void sdi_set_direction( LineDirection dir );

/*
Initialize the module.
uint8_t*        dport    = Port address of data line direction register
uint8_t*        port     = Port address for data line
uint8_t         pin      = Pin # for data line
*/
inline void sdi_init( volatile uint8_t* dport, volatile uint8_t* oport, volatile uint8_t* iport , uint8_t pin )
{
        sdi_dport = dport;
        sdi_oport  = oport;
        sdi_iport  = iport;
        sdi_pin   = pin;
        sdi_set_direction( OUTPUT );

        //Set the data line to marking for 100 ms to clear all garbage and noise
        sdi_set( FALSE );
        _delay_ms( 50 );
        
}

/*
Set the data line direction: input/output
*/
inline void sdi_set_direction( LineDirection dir )
{
         switch( dir )
         {
                case INPUT:
                        *sdi_dport &= ~_BV( sdi_pin );
                        //pinMode( 13, INPUT );
                        return;
                case OUTPUT:
                        //pinMode( 13, OUTPUT );
                        *sdi_dport |= _BV( sdi_pin );
                        return;
                        
         }
}

/*
Set the data line's state
*/
inline void sdi_set( Bool state )
{
        if( state )
                *sdi_oport |= _BV( sdi_pin );
        else
                *sdi_oport &= ~_BV( sdi_pin );
}

/*
Read the data line's state
*/
inline Bool sdi_get()
{
        if ( *sdi_iport & _BV( sdi_pin ) ) return TRUE;
        return FALSE;
}

/*
Sets the data line to marking ( 0 - 1.0 volts )
# SDI12 uses negative logic.
*/
inline void sdi_mark()
{
        //Clear the bit since SDI12 uses reverse logic
        sdi_set( FALSE );
        _delay_us( SPACING );
}

/*
Sets the data line to flat/space ( 3.5 - 5.5 volts )
*/
inline void sdi_space()
{
        //Set the bit
        sdi_set( TRUE );
        _delay_us( SPACING );
}

/*
Wait for the data line to be marking
*/
inline void sdi_wait_mark()
{
        //Wait while pin is HIGH
        while( sdi_get() );
        return;
}

/*
Wait for the data line to be unmarked.
*/
inline void sdi_wait_space()
{
        //Wait while pin is low
        while( !sdi_get() );
        return;
}

/*
Wait for a bit's spacing to pass
*/
inline void sdi_wait_bit()
{
        _delay_us( SPACING );
        return;
}

/*
Read a char
*/
inline uint8_t sdi_read()
{
        uint8_t ch = 0;

        //15ms is max allowed timing between request and response: wait for the start bit
        unsigned long count = 15000;
        while( !sdi_get() && count-- )
                _delay_us( 1 );
                
        //If count == 0, response didn't come, return 0
        if( !count )
                return 0;
                
        
        //Wait for 1/2 bit spacing to set sample in the middle of start bit
        _delay_us( SPACING / 2 );
        
        //Read all data bits
        uint8_t mask = 1;
        for( ; mask < 0x80 ; mask <<= 1 )
        {
                //Wait for the middle of the next bit
                sdi_wait_bit();
                
                if( !sdi_get() )
                        ch |= mask;
                        
        }
        
        //Skip the stop and parity bits
        _delay_us( SPACING*2 );

        //Otherwise return ch
        return ch;
}

/*
Write a char
*/
inline void sdi_write( uint8_t ch )
{
        //Write the start bit
        sdi_set( TRUE ); //Start bit
        _delay_us( SPACING );
        
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
        _delay_us( SPACING );
        
        return;
}

/*
Send a command and retrieve the response. 'command' must be null terminated.
*/
inline Bool sdi_exchange( uint8_t* command, uint8_t* response )
{
          
        //Generate break ( 12ms of space )
        sdi_set_direction( OUTPUT );
        sdi_set( TRUE );
        _delay_us( BREAK_DELAY );
        sdi_set( FALSE );
        _delay_us( POST_BREAK_DELAY );

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
                //if( response[ idx ] == 0 || response[ idx ] == 127 )
                //  return FALSE;
                  
                idx++;
        }
        response[MAX_READ - 1] = 0; //Null terminate
        
        return TRUE;
}

#endif //_SDI_H_
