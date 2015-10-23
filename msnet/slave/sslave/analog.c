#include "analog.h"

void analogEnableCustom(unsigned char pscale, unsigned char vref)
{

  //Clear the power reduction bit of PRR
  PRR &= ~_BV(PRADC);

  //Set the prefered voltage reference
  ADMUX |= vref;

  //Set the prescaler bits in ADCSRA
  ADCSRA |= pscale;       
          
  //If the internal voltage source is used give it some time to start up
  if(vref == ADCREF_INT)
    _delay_ms(10);
    
  //Set the ADC Enable bit in ADCSRA to enable the device
  ADCSRA |= _BV(ADEN);
}


void analogEnableDefault()
{
  analogEnableCustom(PS_64, ADCREF_VCC);
}


void analogDisable()
{
  PRR     |= _BV(PRADC);
  ADCSRA   &= ~_BV(ADEN);
}


unsigned int analogGet(unsigned int c)
{
    
  //Clear the channel select bits of ADMUX
  ADMUX &= 0b11000000;
  
  //Refill the channel bits with the appropriate channel
  ADMUX |= c;
  
  //Start the conversion by setting the ADSC bit of ADCSRA
  ADCSRA |= _BV(ADSC);
        
  //Wait for the converaion to finish
  while((ADCSRA >> ADSC) & 0x1);
  
  //Must read ADCL before reading ADCH to prevent write locking the registers
  unsigned int low = ADCL;
  unsigned int high = ADCH;
  return high << 8 | low;
}


float analogGetScaled(unsigned int c, float vref)
{
  //Read from the channel
  unsigned int v = analogGet(c);
  
  //Scale the value and return
  //1024 is the maximum value of a 10bit analog read
  return (vref / 1024.0) * v;
}

float getVoltage(unsigned int c, float vref)
{
  return (analogGet(c) * vref) / 1024.0;
}

