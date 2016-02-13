#include "sdi.h"
#include <stdint.h>

void setup()
{
        Serial.begin( 9600 );
        delay( 100 );

        Serial.print( "Initializing sdi..." );
        sdi_init( &DDRB, &PORTB, 5 );
        Serial.println( "ok." );
        
        uint8_t response[75] = {0};
        uint8_t* command = ( uint8_t* ) "0!";

        Serial.print( "Initializing exchange..." );
        Bool test = sdi_exchange( (uint8_t*)command, (uint8_t*)response );
        Serial.println( "exchange complete." );

        Serial.print( "readcount = " ); Serial.println( readcount );
        Serial.print( "writecount = " ); Serial.println( writecount );
        Serial.print( "tracker = " ); Serial.println( tracker );
        Serial.print( "rbitscount = " ); Serial.println( rbitscount );
        Serial.println( "dbg = " );
        for( int i = 0 ; i < dbgidx ; i++ )
          Serial.println( dbg[i], BIN );
        Serial.print( "Response: " );
        int i = 0;
        for( ; response[i] != 0 ; i++ )
                Serial.println( response[i] );
}

void loop()
{
    /*sdi_set( TRUE );
    delayMicroseconds( 830 );
    sdi_set( FALSE );
    delayMicroseconds( 830 );*/
    
    //sdi_write( '0' );
}
