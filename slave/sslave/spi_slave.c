#include "spi_slave.h"
//#include <ioavr.h>
//#include <inavr.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/crc16.h>

unsigned char outgoing_packet[MAX_PACKET_LENGTH];
unsigned char incoming_packet[MAX_PACKET_LENGTH];

unsigned char storedUSIDR;
unsigned char numReceived;

volatile struct usidriverStatus_t spiX_status;

/*! \brief  Timer/Counter 0 Compare Match Interrupt handler.
 *
 *  This interrupt handler is only enabled when transferring data
 *  in master mode. It toggles the USI clock pin, i.e. two interrupts
 *  results in one clock period on the clock pin and for the USI counter.
 */
ISR(TIM0_COMPA_vect)
{
  USICR |= (1<<USITC);
}



/*! \brief  USI Timer Overflow Interrupt handler.
 *
 *  This handler disables the compare match interrupt if in master mode.
 *  When the USI counter overflows, a byte has been transferred, and we
 *  have to stop the timer tick.
 *  For all modes the USIDR contents are stored and flags are updated.
 */
ISR(USI_OVF_vect)
{
  // Master must now disable the compare match interrupt
  // to prevent more USI counter clocks.
  if( spiX_status.masterMode == 1 ) {
    TIMERMASK &= ~(1<<OCIE0A); //changed TIMSK from TIMSK0
  }

  // Update flags and clear USI counter
  USISR = (1<<USIOIF);
  spiX_status.transferComplete = 1;

  // Copy USIDR to buffer to prevent overwrite on next transfer.
  storedUSIDR = USIDR;
}

/*
*
* This handler listens for changes to the slave select pin. High indicates the slave is
* not selected, low means selected. Simply modify a global boolean, which the software loop
* must check for before listening for communication on USI
*/
/*  ---Not in use!---
//  ---Yes, you can delete it at some point.---
//  ---Caused a bad exchange.---
ISR(PCINT0_vect)
{
  if(spiX_status.masterMode == 0) {
    if((USI_IN_REG & (1<<SPI_SS_PIN))) //extract the value of Slave select (if 0, then line is low and we're selected)
      spiX_status.slaveSelected = 0;
    else
      spiX_status.slaveSelected = 1;
    if(spiX_status.slaveSelected){
      usi_enable();
    }
    else{
      usi_disable();
    }
  }
}
*/

/*
 * To enable the USI, we set the USI direction pin (i.e. DATAOUT/MISO) to output.
 * This allows us to keep the USI module running.
 */
void usi_enable()
{
  USI_DIR_REG |= (1<<USI_DATAOUT_PIN);

}

/*
 * To disable the USI, we set the USI direction pin (i.e. DATAOUT/MISO) to input
 * This allows us to keep the USI module running even if this slave is deselected.
 */
void usi_disable()
{
   USI_DIR_REG &= ~(1<<USI_DATAOUT_PIN);
}


void spiX_initslave( char spi_mode )
{
  // Configure port directions. NOTE: the USI_DATAOUT_PIN is not enabled here because it is controlled by the slave select
    USI_DIR_REG = (1<<USI_DATAOUT_PIN);                      // Outputs.
  USI_DIR_REG &= ~((1<<USI_DATAIN_PIN) | (1<<USI_CLOCK_PIN) | (1<<SPI_SS_PIN));// Inputs.
  USI_OUT_REG |= (1<<USI_DATAIN_PIN) | (1<<USI_CLOCK_PIN) ;  // Pull-ups. We don't want to configure SS pin as a pullup
    //PINCHANGE |= (1<<SPI_SS_PIN);  //configure interrupt for slave select
  //GIMSK |= (1<<PINFLAG);        //enable interrupt for slave select

  // Configure USI to 3-wire slave mode with overflow interrupt.
  USICR = (1<<USIWM0) | (1<<USIOIE) |
          (1<<USICS1) | (spi_mode<<USICS0);

  // Init driver status register.
  spiX_status.masterMode       = 0;
  spiX_status.transferComplete = 0;
  spiX_status.writeCollision   = 0;
    spiX_status.slaveSelected    = 0;
    spiX_status.receiving        = 1; //we always start out in the receiving state

  storedUSIDR = 0;
}




char spiX_put( unsigned char val )
{
  // Check if transmission in progress,
  // i.e. USI counter unequal to zero.
  if( (USISR & 0x0F) != 0 ) {
    // Indicate write collision and return.
    spiX_status.writeCollision = 1;
    return 0;
  }

  // Reinit flags.
  spiX_status.transferComplete = 0;
  spiX_status.writeCollision = 0;

  // Put data in USI data register.
  USIDR = val;
  // Master should now enable compare match interrupts.
  if( spiX_status.masterMode == 1 ) {
    TIMERFLAG |= (1<<OCF0A);   // Clear compare match flag.
    TIMERMASK |= (1<<OCIE0A); // Enable compare match interrupt.
  }
  if( spiX_status.writeCollision == 0 ) return 1;
  return 0;
}



unsigned char spiX_get()
{
  return storedUSIDR;
}



void spiX_wait()
{
  do {} while( spiX_status.transferComplete == 0 );
}



void send_packet()
{
  //Indicate start of packet with '$'
  while(!spiX_put(RECEIVE_CHAR)){}
  spiX_wait();
  //send another start of packet char
  while(!spiX_put(RECEIVE_CHAR)){}
  spiX_wait();
  int index = 0;
  while(index < MAX_PACKET_LENGTH && index < outgoing_packet[0])
  {
   while(!spiX_put(outgoing_packet[index])){}; //continue trying to put char even if there are collisions
   spiX_wait(); //wait for byte to be sent
   index++;
  }

  if(index == MAX_PACKET_LENGTH) return;
  //finally send '#'
  while(!spiX_put(outgoing_packet[index])){}
  spiX_wait();
  spiX_status.receiving = 1;
}


void receive_packet()
{
  unsigned char buff;
  int index = 0;
  //tell master we're ready (wait until master starts sending packet)
  while(1)
  {
    spiX_wait();
    buff = spiX_get();
    if(buff != RECEIVE_CHAR) break;

    while(!spiX_put('!')){}
  }

  //store buff, then receive rest of packet
  incoming_packet[0] = buff;
  index++;
  numReceived = 1;
  spiX_put(3);
 while(index < MAX_PACKET_LENGTH)
 {
  spiX_wait(); //wait for a new byte to be ready
  buff = spiX_get();
  spiX_put(3);
  incoming_packet[index] = buff;
  numReceived++;
  if(index == incoming_packet[0]) break; //read all length bytes (stored in incoming_packet[0])
  index++;

 }
 spiX_status.receiving = 0;
}


int prepare_packet(char* payload, int length)
{
  if(length > MAX_PACKET_LENGTH-5) return 0;

  //fill in outgoing_packet
  outgoing_packet[0] = length+5;

  //place payload
  int i;
  for(i=0;i<length;i++)
  {
    outgoing_packet[i+1] = payload[i];
  }

  //add end of message delimiter
  outgoing_packet[length+1] = 0;

  //compute CRC, place in outgoing packet
  uint16_t crc = 0xffff;  //crc-16 uses 0xffff as initial value
  for(i = 0; i < length+1;i++)
  {
   crc = _crc16_update(crc,outgoing_packet[i]);
  }
  outgoing_packet[length+2] = crc & 0xff; //get the lower 8 bits
  outgoing_packet[length+3] = crc >> 8;   //get the upper 8 bits

  //end packet with #
  outgoing_packet[length+4] = '#';

  return 1;
}



int check_integrity()
{
  int length = (int) incoming_packet[0];
  uint16_t crc = 0xffff;  //crc-16 uses 0xffff as initial value
  int i;
  for(i = 0; i < length-4;i++)
  {
   crc = _crc16_update(crc,incoming_packet[i]);
  }

  //compare listed crc with found crc
  uint16_t found_crc = incoming_packet[length-3] | incoming_packet[length-2] << 8;

  if(found_crc == crc) return 1;
  return 0;

}

int slave_selected(){
  if((USI_IN_REG & (1<<SPI_SS_PIN))) //extract the value of Slave select (if 0, then line is low and we're selected)
    spiX_status.slaveSelected = 0;
  else
    spiX_status.slaveSelected = 1;
  if(spiX_status.slaveSelected){
    usi_enable();
  }
  else{
    usi_disable();
    while(!spiX_put(0)){}
  }
        
        return spiX_status.slaveSelected;
}


