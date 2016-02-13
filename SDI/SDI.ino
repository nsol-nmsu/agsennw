#include "sdi.h"
#include <stdint.h>

void setup()
{
        Serial.begin( 9600 );
        delay( 100 );

        Serial.print( "Initializing sdi..." );
        sdi_init( &DDRB, &PORTB, 5 );
        sdi_set_direction( OUTPUT );
        Serial.println( "ok." );
        
        uint8_t response[75] = {0};
        uint8_t* command = ( uint8_t* ) "0!";

        Serial.print( "Initializing exchange..." );
        Bool test = sdi_exchange( (uint8_t*)command, (uint8_t*)response );
        Serial.println( "exchange complete." );

        Serial.print( "Response: " );
        while( response != 0 )
                Serial.println( *response );
}

void loop()
{
}
