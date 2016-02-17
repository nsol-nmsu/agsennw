#ifndef __ANALOG_H__
#define __ANALOG_H__

#include <avr/io.h>
#include <util/delay.h>

//Prescaler bits patterns, (F_ADC = F_CPU / XXX) in PS_XXX
#define PS_128  7   //00000111
#define PS_64   6   //00000110
#define PS_32   5
#define PS_16   4
#define PS_8    3
#define PS_4    2
#define PS_2    1

//ADC Stuff
#define ADC_CHANNEL_COUNT 8
#define ADCREF_VCC 0b00000000
#define ADCREF_EXT 0b01000000
#define ADCREF_INT 0b10000000

#define ADC_0     0x0
#define ADC_1     0x1
#define ADC_2     0x2
#define ADC_3     0x3
#define ADC_4     0x4
#define ADC_5     0x5
#define ADC_6     0x6
#define ADC_7     0x7
#define ADC_TEMP  0b00100010




/*
Enables the ADC with a custom prescaler and voltage reference.
This function clears the PRADC (Power Reduction Bit) of PRR and 
modifies several bits in ADCSRA and ADMUX.
*/
void analogEnableCustom(unsigned char pscale, unsigned char vref);

/*
Uses analogEnableCustom() to enable the ADC with default settings
*/
void analogEnableDefault();

/*
Disables the ADC.  Sets the PRADC bit of PRR and clears the ADEN bit of ADCSRA
*/
void analogDisable();

/*
Reads a value from the given analog channel,
if the given channel does not exist 0 is returned
*/
unsigned int analogGet(unsigned int c);

/*
Provided the actual voltage of the ADC's reference source 
this function will read a value from the ADC channel 'c' 
and scale it to voltage.
*/
float analogGetScaled(unsigned int c, float vref);

/*
Returns the aproximate analog given VREF
*/
float getVoltage(unsigned int c, float vref);


#endif //__ANALOG_H
