#ifndef _TTEMP_SLAVE_C_
#define _TTEMP_SLAVE_C_

#include "slave.h"
#include "analog.h"
#include <util/delay.h>

void sslave_init()
{
  analogEnableDefault();
}

float extr_temp()
{
  return 50.0 + (getVoltage(ADC_3, 5.0) - 2.5) * 100.0;
}

Slave sslave =
{
  .id = 102,
  .type = 1,
  .name = {'t','t','e','m','p'},
  .m = 10, .d = 19, .y = 15,
  .rcount = 1,
  .wcount = 0,
  .vref = 5.0,
  .rchans[0] = extr_temp,
  .units[0]  = 1,
  .info = {'h', 'e', 'l', 'l', 'o'}
};
#endif //_TTEMP_SLAVE_C_
