/*
This file contains the declarations necessary for any valid slave implementation.
A slave implementation must implement all units declared in this file, including variables.
*/

#ifndef _SLAVE_H_
#define _SLAVE_H_

int   slave_id;                    //Unique slave id
char* slave_type;                  //Slave type string
char* slave_name;                  //Slave name
char  slave_init_date[3];          //[day, month, year] where year is A.C - 2000
char  slave_rcount;                //Readable channel count
char  slave_wcount;                //Writable channel count
char* slave_info;                  //Other information, description, etc.

//Initializes the slave if initialization is necessary, returns non-zero failure
int slave_init();

//Initiates a measurement, returns time until complete
unsigned slave_measure();

//run measurments
void slave_run_measure();

//Reads a measurment, returns measurment as string
char* slave_read( unsigned ch );

//Writes data to some channel, returns non-zero on failure
int slave_write( const char* msg, unsigned ch );

//If write opperations is buffered, this function applies the changes,
// causing all writes to take effect.  Return non-zero on failure.
int slave_apply();

#endif //_SLAVE_H_
