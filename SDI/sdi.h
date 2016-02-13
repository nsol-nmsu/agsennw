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
#include <Arduino.h>

#ifndef F_CPU
#error sdi12.h requires a definition for F_CPU, none found.
#endif

//Timing is in micro seconds
#define MAX_RESPONSE_DELAY      15000
#define BREAK_DELAY             12000
#define POST_BREAK_DELAY        8330
#define SPACING                 830  // Microseconds
#define MAX_READ                75
#define CYCLES_FOR_15MS         ( F_CPU * 15UL / 1000UL )
#define CYCLES_PER_BIT_WAIT     10 //Just a guess, not tested nor calculated

//Use macros for valeus since ArduinoStudio screws up C enums
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
static volatile uint8_t* sdi_port;  //IO port
static volatile uint8_t  sdi_pin;   //Pin

//Debug
int readcount = 0;
int writecount = 0;
int tracker = 0;
int rbitscount = 0;
uint8_t dbg[10] = {0};
int dbgidx = 0;

inline void sdi_set( Bool state );
inline void sdi_set_direction( LineDirection dir );

/*
Initialize the module.
uint8_t*        dport    = Port address of data line direction register
uint8_t*        port     = Port address for data line
uint8_t         pin      = Pin # for data line
*/
inline void sdi_init( volatile uint8_t* dport, volatile uint8_t* port, uint8_t pin )
{
        sdi_dport = dport;
        sdi_port  = port;
        sdi_pin   = pin;
        sdi_set_direction( OUTPUT );

        //Set the data line to marking for 100 ms to clear all garbage and noise
        sdi_set( FALSE );
        _delay_ms( 100 );
        
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
                *sdi_port |= _BV( sdi_pin );
        else
                *sdi_port &= ~_BV( sdi_pin );
}

/*
Read the data line's state
*/
inline Bool sdi_get()
{
        return ( *sdi_port & _BV( sdi_pin ) )? TRUE : FALSE;
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
        tracker = 0;
        uint8_t ch = 0;

        //15ms is max allowed timing between request and response: wait for the start bit
        unsigned long count = 15000;
        while( !sdi_get() && count-- )
                _delay_us( 1 );
                
        //If count == 0, response didn't come, return 0
        if( !count )
                return 0;

        tracker++; //1
        
        //Wait for the start bit to end
        sdi_wait_bit();

        tracker++;//2
        
        //Wait for 1/2 bit spacing to set sample in the middle of bit 0
        _delay_us( SPACING / 2 );

        tracker++;//3
        
        //Read all data bits
        uint8_t mask = 1;
        for( ; mask < 0x80 ; mask <<= 1 )
        {
                if( !sdi_get() )
                        ch |= mask;
                else
                        ch &= ~mask;
                        
                //Wait for the middle of the next bit
                sdi_wait_bit();
                rbitscount++;
        }

        dbg[dbgidx++] = ch;

        tracker++;//4
        
        //Should now be in the middle of the parity bit, returns 0 if it is bad
        Bool aparity = sdi_get();
        Bool cparity = parity_even_bit( ch );

        tracker++;//5
        
        //Wait for stop bit to pass
        _delay_us( SPACING / 2 );
        sdi_wait_bit();

        tracker++;//6
        
        //If correct parity and acctual parity are different then return 0
        if( !cparity != !aparity )
                return 0;

        tracker++;//7
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
        
        //Wait for sensors to wake up
        //_delay_ms( 100 );

        //Write all data
        uint8_t* c;
        for( c = command ; *c != 0 ; c++ )
        {
                sdi_write( *c );
                writecount++;
        }
        
        
        sdi_set_direction( INPUT );
        
        //Read all input until <CR><LF>
        int idx = 0;
        while( !( idx > 1 && response[ idx - 2 ] == '\r' && response[ idx - 1 ] == '\n' )
                && idx <= MAX_READ )
        {
                response[ idx ] = sdi_read();
                if( response[ idx ] == 0 )
                  return FALSE;

                idx++;
                readcount++;
        }
        
        return TRUE;
}

#endif //_SDI_H_
