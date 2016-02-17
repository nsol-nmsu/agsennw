/*
This file contains the structure definitions for Slave and SlavePersist.
The Slave structure consists of members used to keep track of the current
state of the slave, while the SlavePersist structure is smaller and only
houses the data that should be kept in EEPROM.
*/

#ifndef _SLAVE_H_
#define _SLAVE_H_

#define MAX_NAME_LEN 10
#define MAX_INFO_LEN 30
#define MAX_CHANNELS 10

typedef struct
{
  int  id;                        //Id
  int  type;                      //Sensor type
  char name[MAX_NAME_LEN];        //Name of this sensor
  char m, d, y;                   //Deployment date, y is in years since 2000
  char rcount;                    //Number of read channels
  char wcount;                    //Number of write channels
  char ilen;                      //Length of info
  char info[MAX_INFO_LEN];       //Any additional info that associated with the
                                  //node, maybe a description
}Slave;

/*
The slave_xxx.h file included is also expected to define:

//Initializes the slave if initialization is necessary, returns non-zero failure
int slave_init();

//Begins a measurment on channel ch, returns time in ms until measurment is ready
unsigned slave_measure( unsigned ch );

//Reads a measurment, returns measurment as string
char* slave_read( unsigned ch );

//Writes data to some channel, returns non-zero on failure
int slave_write( const char* msg, unsigned ch );

//If write opperations is buffered, this function applies the changes,
// causing all writes to take effect.  Return non-zero on failure.
int apply( unsigned ch );
*/
#endif //_SLAVE_H_
