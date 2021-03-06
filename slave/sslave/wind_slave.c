/**
A 03101 Wind Anemometer for sensing wind speeds.
**/
#include "analog.h"
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define HI 0x0F     //bit value 0001111 to simulate low crossing to high
#define LO 0x70     //bit value 1110000 to simulate high crossing to low

// only for reference, with the NSOL slave boards, 03101 typically has
// a threshold reading of 495
static int THRESH = 495;
static float wind_speed = 0;

// Calculate thresh value at start up once
void slave_init()
{
  analogEnableCustom(PS_128, ADCREF_VCC);
   int max = 0;
   int min = 1024;
   int current = 0;
   int j = 9000;

   while(j--){
      current = analogGet(ADC_1);
      max = (current > max)? current : max;
      min = (current < min)? current : min;
   }

   THRESH = (max + min) / 2;
}
// Give run_measure a 2 second window to calculate wind speed
unsigned slave_measure( unsigned ch )
{
   return 2000;
}
/*
*  Zero crossing algorithm to calculate the frequency of the given
*  voltage in ADC_1 and use it to calculate wind speeds (U) using the given
*  expression U = MX + B, where M = multiplier, X = Hertz, B = Offset
*/ 
void slave_run_measure()
{
   wind_speed               = 0;
   double wave_length       = 0;
   double Hz                = 0;
   uint8_t raw              = 0x55; //difuse byte to avoid false crossing
   unsigned int temp_raw    = 0;
   uint8_t smooth           = 0x55;
   uint8_t cross_flag       = 0;
   uint8_t rx               = 0;
   uint8_t sx               = 0;
   int i                    = 9000;
   unsigned int count       = 0;

   while(i--){
      rx = (analogGet(ADC_1) > THRESH)? 1 : 0;
      raw = ((raw << 1) | rx) & ((1 << 7) - 1);
      temp_raw = raw;

      int bit_count;
      bit_count = __builtin_popcount(temp_raw);

      sx = (bit_count > 3)? 1 : 0;

      smooth = ((smooth << 1) | sx) & ((1 << 7) - 1);

      switch(smooth){
         case LO:
         case HI:
                  if(cross_flag++ > 1){
                     wave_length += count;
                     // Note: this expression best fits the behavior
                     // of Hz given the slave runs at 8MHz with no defined
                     // delay between samples counted, (subject to change)
                     Hz = 0.003138664 * pow((-(wave_length - 1485865)/wave_length), (5000000.0/5020055.0));
                     wind_speed = 1.677 * Hz + 0.4; // convertion to mph
                     return;
                  }
                  wave_length = count;
                  count = 0;
                  break;
         default:
                  count++;
      }
   }

   wind_speed = 0;
   return;
}

char* slave_read( unsigned ch )
{
   static char str[20];
   int i = 0;
   str[i++] = 'w'; str[i++] = 'i'; str[i++] = 'n'; str[i++] = 'd';
   str[i++] = ':';
   dtostrf(wind_speed, 5,1,&str[i]);
   i += 5;
   str[i++] = ':';
   str[i++] = 'm';
   str[i++] = 'p';
   str[i++] = 'h';
   str[i++] = '\n';
   str[i++] = 0;

   return str;
};

int slave_write( const char* msg, unsigned ch )
{
        return 1;
}

int slave_apply( unsigned ch )
{
        return 1;
}

int slave_id = 105;
char* slave_type = "anemo";
char* slave_name = "itemp";
char slave_init_date[3] = {27,2,16};
char slave_rcount = 1;
char slave_wcount = 0;
char* slave_info = "03101 Wind Anemometer";
