/**
A 03101 Wind Anemometer for sensing wind speeds.
**/
#include "analog.h"
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define THRESH 499 //499 ADC_1 thresh, any other would be 0
#define HI 0x0F     //bit value 00111
#define LO 0x70     //bit value 11000,

void slave_init()
{
  analogEnableCustom(PS_128, ADCREF_VCC);
}

unsigned slave_measure( unsigned ch )
{
   return 0;
}

int slave_run_measure()
{
   return 0;
}

char* slave_read( unsigned ch )
{
   double wave_length = 0;
   double result = 0;
   double Hz = 0;
   uint8_t raw = 0x55; //difuse byte to avoid false crossing
   unsigned int temp_raw = 0;
   uint8_t smooth = 0x55;
   uint8_t cross_flag = 0;
   uint8_t rx = 0;
   uint8_t sx = 0;
   int i = 1000;
   unsigned int count = 0;
   static char str[15];

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
                  wave_length = (2.0 * count);

                  if(cross_flag++ > 1){
                     Hz = 0.4 * pow((-(wave_length - 1550)/wave_length), (25.0/26.0));
                     result = 1.677 * Hz + 0.4;
                     dtostrf(Hz, 7,3,str);
                     return str;
                  }
                  count = 0;
                  break;
         default:
                  count++;
      }
   }

   dtostrf(result, 7,3,str);
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
char* slave_type = "Anemometer";
char* slave_name = "itemp";
char slave_init_date[3] = {27,2,16};
char slave_rcount = 1;
char slave_wcount = 0;
char* slave_info = "03101 Wind Anemometer";

