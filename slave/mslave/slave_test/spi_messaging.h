/*
A few messaging functions to help communicate with the spi slaves
*/

#ifndef _SPI_MESSAGING_H_
#define _SPI_MESSAGING_H_

#include "spi_master.h"
#include <string.h>
#include <Arduino.h>

#define NUM_RETRIES     10
#define ECHO_TIMEOUT    1000
#define READ_TIMEOUT    20000
#define MEASURE_TIMEOUT 3000
#define MSG_TIMEOUT     1000

#define MAX_NAME_LEN 10
#define MAX_INFO_LEN 30

void message_init(char ss_pins[], char sscount);

int       get_id(unsigned int ss);
char*     get_type(unsigned int ss);
char*     get_name(unsigned int ss);
int       get_rcount(unsigned int ss);
int       get_wcount(unsigned int ss);
int       slave_measure( unsigned int ss );
char*     slave_read(unsigned int ss, unsigned char chan);
char*     get_info(unsigned int ss);

bool    is_slave(unsigned int ss);
bool    set_type(unsigned int ss, int type);
bool    set_name(unsigned int ss, char name[MAX_NAME_LEN]);
bool    set_info(unsigned int ss, char info[MAX_INFO_LEN]);
bool    write_chan(unsigned int ss, unsigned char chan, char* msg);


#endif //_SPI_MESSAGING_H_
