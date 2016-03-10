#ifndef _TIME_H_
#define _TIME_H_
#include <TimerOne.h>

//Keeps track of time in deciseconds

static volatile unsigned long timer = 0; //Time in deciseconds
static char isStarted = 0;

static inline void update_time()
{
  timer++;
}

inline void time_init()
{
  if( isStarted ) return;

  isStarted = 1;
  Timer1.initialize(100000); //Update timecount every 100ms (a.k.a 1 decisecond )
  Timer1.attachInterrupt(update_time);
}

inline void time_kill()
{
  if( !isStarted ) return;

  Timer1.stop();
  Timer1.detachInterrupt();
}

inline unsigned long time_get()
{
  unsigned long temp;
  noInterrupts();
  temp = timer;
  interrupts();
  return temp;
}

#endif
