/*
A few messaging functions to help communicate with the spi slaves
*/

#ifndef _SPI_MESSAGING_H_
#define _SPI_MESSAGING_H_

#define NUM_RETRIES  100
#define MSG_TIMEOUT  1000

#define MAX_NAME_LEN 10
#define MAX_INFO_LEN 30

void message_init(char ss_pins[], char sscount);

int       get_id(unsigned int ss);
int       get_type(unsigned int ss);
char*     get_name(unsigned int ss);
int       get_rcount(unsigned int ss);
int       get_wcount(unsigned int ss);
int       measure_chan( unsigned int ss, unsigned int chan);
char*     read_chan(unsigned int ss, unsigned int chan); 
char*     get_info(unsigned int ss);

bool    is_slave(unsigned int ss);
bool    set_type(unsigned int ss, int type);
bool    set_name(unsigned int ss, char name[MAX_NAME_LEN]);
bool    set_info(unsigned int ss, char info[MAX_INFO_LEN]);
bool    write_chan(unsigned int ss, unsigned char chan, char* msg);


#endif //_SPI_MESSAGING_H_
