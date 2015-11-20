#include "spi_master.h"
#include "spi_messaging.h"
#include <string.h>

//Helpers
static inline int to_int(unsigned char bs[2]);
static inline float to_float(unsigned char bs[4]);
static inline char* from_int(const int* i);
static inline char* from_float(const float* f);

void message_init(char ss_pins[], char sscount)
{
  SPI_setup();

  for(int i = 0 ; i < sscount ; i++)
  {
    pinMode(ss_pins[i], OUTPUT);
    digitalWrite(ss_pins[i], HIGH);
  }
}

int get_id(unsigned int ss)
{
  if(!exchange(ss, "i", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    return to_int(&incoming_packet[1]);
  }
}

int get_type(unsigned int ss)
{
  if(!exchange(ss, "t", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    return to_int(&incoming_packet[1]);
  }
}

char* get_name(unsigned int ss)
{
  static char name_buf[MAX_NAME_LEN];
  
  if(!exchange(ss, "n", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return NULL;
  }
  else
  {
    memcpy(name_buf, &incoming_packet[1], MAX_NAME_LEN);
    return name_buf;
  }
}

int get_rcount(unsigned int ss)
{
  if(!exchange(ss, "r", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    return to_int(&incoming_packet[1]);
  }
}

int       get_wcount(unsigned int ss)
{
  if(!exchange(ss, "w", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    return to_int(&incoming_packet[1]);
  }
}

float read_chan(unsigned int ss, unsigned int chan)
{
  if(!exchange(ss, "q", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    unsigned char* data = &incoming_packet[1];
    if(data[0] == 'e' && data[1] == 'r' && data[2] == 'e')
      return 0;
    else
      return to_float(data);
  }
}

float read_volts(unsigned int ss, unsigned int adc_pin)
{
  if(!exchange(ss, "v", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    unsigned char* data = &incoming_packet[1];
    if(data[0] == 'e' && data[1] == 'r' && data[2] == 'e')
      return 0;
    else
      return to_float(data);
  }
}

int get_unit(unsigned int ss, unsigned int chan)
{
  if(!exchange(ss, "u", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    unsigned char* data = &incoming_packet[1];
    if(data[0] == 'e' && data[1] == 'r' && data[2] == 'e')
      return 0;
    else
      return to_int(data);
  }
}

char* get_info(unsigned int ss)
{
  static char info_buf[MAX_INFO_LEN];
  
  if(!exchange(ss, "n", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return NULL;
  }
  else
  {
    memcpy(info_buf, &incoming_packet[1], MAX_INFO_LEN);
    return info_buf;
  }
}

bool is_slave(unsigned int ss)
{
  if(!exchange(ss, "e", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool set_type(unsigned int ss, int type)
{
  char com[3];
  com [0] = 'T';
  memcpy(&com[1], from_int(&type), 2);
  
  if(!exchange(ss, com, 3, NUM_RETRIES, MSG_TIMEOUT))
  {
    return false;
  }
  else
  {
    unsigned char* data = &incoming_packet[1];
    if(data[0] == 'o' && data[1] == 'k')
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool set_name(unsigned int ss, char name[MAX_NAME_LEN])
{
  char com[MAX_NAME_LEN + 1];
  com[0] = 'N';
  memcpy(&com[1], name, MAX_NAME_LEN);
  
  if(!exchange(ss, com, MAX_NAME_LEN + 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return false;
  }
  else
  {
    unsigned char* data = &incoming_packet[1];
    if(data[0] == 'o' && data[1] == 'k')
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool set_info(unsigned int ss, char info[MAX_INFO_LEN])
{
  char com[MAX_INFO_LEN + 1];
  com[0] = 'F';
  memcpy(&com[1], info, MAX_INFO_LEN);
  
  if(!exchange(ss, com, MAX_INFO_LEN + 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return false;
  }
  else
  {
    unsigned char* data = &incoming_packet[1];
    if(data[0] == 'o' && data[1] == 'k')
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool set_unit(unsigned int ss, unsigned int chan, int unit)
{
  char com[3];
  com [0] = 'U';
  memcpy(&com[1], from_int(&unit), 2);
  
  if(!exchange(ss, com, 3, NUM_RETRIES, MSG_TIMEOUT))
  {
    return false;
  }
  else
  {
    unsigned char* data = &incoming_packet[1];
    if(data[0] == 'o' && data[1] == 'k')
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

bool write_chan(unsigned int ss, unsigned int chan, float value)
{
  char com[5];
  com [0] = 'W';
  memcpy(&com[1], from_float(&value), 4);
  
  if(!exchange(ss, com, 5, NUM_RETRIES, MSG_TIMEOUT))
  {
    return false;
  }
  else
  {
    unsigned char* data = &incoming_packet[1];
    if(data[0] == 'o' && data[1] == 'k')
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

union
{
  float f;
  int i;
  char bs[4];
}convert;

static inline int to_int(unsigned char bs[2])
{
  convert.bs[0] = bs[0];
  convert.bs[1] = bs[1];
  return convert.i;
}

static inline float to_float(unsigned char bs[4])
{
  convert.bs[0] = bs[0];
  convert.bs[1] = bs[1];
  convert.bs[2] = bs[2];
  convert.bs[3] = bs[3];
  return convert.f;
}

static inline char* from_int(const int* i)
{
  return (char*)i;
}

static inline char* from_float(const float* f)
{
  return (char*) f;
}