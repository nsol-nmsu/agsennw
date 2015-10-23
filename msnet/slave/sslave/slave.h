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

//Pack these structures to save space
#pragma push(1)

typedef struct
{
  int  id;                        //Id
  int  type;                      //Sensor type
  char name[MAX_NAME_LEN];        //Name of this sensor
  char m, d, y;                   //Deployment date, y is in years since 2000
  char rcount;                    //Number of read channels
  char wcount;                    //Number of write channels
  
  //Voltage reference to use for ADC voltage conversion
  float vref;
  
  //Pointers to functions that will compute the value for each channel
  float (*rchans[MAX_CHANNELS])();
  
  //Pointers to functions that will write to each channel
    //if the channel is not writable then the channel can be NULL
  void (*wchans[MAX_CHANNELS])(float f);
                    
  int   units[MAX_CHANNELS];      //Units for the values of each channel
  char   info[MAX_INFO_LEN];      //Any additional info that associated with the
                                  //node, maybe a description
}Slave;

typedef struct
{
  int   id;
  int   type;
  char  name[MAX_NAME_LEN];
  char  m, d, y;
  char  rcount;
  char  wcount;
  float vref;
  int   units[MAX_CHANNELS];
  int   info[MAX_INFO_LEN];
}SlavePersist;

#pragma pop
#endif //_SLAVE_H_
