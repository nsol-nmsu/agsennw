#include "slave.h"
#include <stdlib.h>

/**
A dummy slave.  Increments it's output every request.
**/

int slave_init()
{
        return 0;
};

unsigned slave_measure( unsigned ch )
{
        return 0;
};

void slave_run_measure()
{
        return;
};

char* slave_read( unsigned ch )
{
  return "dummy:dummyValue:dummyUnit\n";
};

int slave_write( const char* msg, unsigned ch )
{
        return 1;
};

int slave_apply( unsigned ch )
{
        return 1;
};

int   slave_id = 1;
char* slave_type = "dummy";
char* slave_name = "dummy slave";
char  slave_init_date[3] = { 27, 2, 16 };
char  slave_rcount = 1;
char  slave_wcount = 0;
char* slave_info = "A dummy slave";

