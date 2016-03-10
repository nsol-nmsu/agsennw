#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "slave.h"
#include "spi_slave.h"
#include "sdi.h"
#include <util/delay_basic.h>

int main(void)
{
	sdi_init( &DDRB, &PORTB, &PINB, 0 );
	
	uint8_t response[78];
	DDRB |= _BV(1);
	
	while(1)
	{
	        sdi_exchange((uint8_t*)"1!", response );
	        if(response[0] == '1') PORTB |= _BV(1);
	        _delay_loop_2(0);
	        _delay_loop_2(0);
	        _delay_loop_2(0);
	}
	
	/*while(1)
	{
	        sdi_set(1);
	        //sdi_delay(1);
	        SDI_HALF_SPACE_DELAY
	        sdi_set(0);
	        SDI_HALF_SPACE_DELAY
	        //sdi_delay(1);
	}*/
	return 0;
}
