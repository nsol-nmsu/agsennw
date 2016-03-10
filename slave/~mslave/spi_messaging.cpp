#include "spi_messaging.h"

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
  if(!exchange(ss, (char*)"i", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    return to_int(&incoming_packet[1]);
  }
}

char* get_type(unsigned int ss)
{
  if(!exchange(ss, (char*)"t", 1, NUM_RETRIES, MEASURE_TIMEOUT))
  {
    Serial.println("timeout");
    return 0;
  }
  else
  {
    char* data = (char*)&incoming_packet[1];
    if(data[0] == 'e' && data[1] == 'r' && data[2] == 'e')
    {  
      Serial.println("err");
      return 0;
    }
    else
    {
      Serial.println("noerr");
      return (char*)data;
    }
  }
}

char* get_name(unsigned int ss)
{
  static char name_buf[MAX_NAME_LEN];
  
  if(!exchange(ss, (char*)"n", 1, NUM_RETRIES, MSG_TIMEOUT))
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
  if(!exchange(ss, (char*)"r", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    return to_int(&incoming_packet[1]);
  }
}

int get_wcount(unsigned int ss)
{
  if(!exchange(ss, (char*)"w", 1, NUM_RETRIES, MSG_TIMEOUT))
  {
    return 0;
  }
  else
  {
    return to_int(&incoming_packet[1]);
  }
}

int slave_measure( unsigned int ss)
{
  if(!exchange(ss, (char*)"m", 1, NUM_RETRIES, MEASURE_TIMEOUT))
  {
    Serial.println("timeout");
    return 0;
  }
  else
  {
    char* data = (char*)&incoming_packet[1];
    if(data[0] == 'e' && data[1] == 'r' && data[2] == 'e')
    {  
      Serial.println("err");
      return 0;
    }
    else
    {
      Serial.println("noerr");
      return to_int( (unsigned char*)data );
    }
  }
}

char* slave_read(unsigned int ss, unsigned char chan)
{
  char cmd[2] = { 'q', chan };
  if(!exchange(ss, cmd, 2, NUM_RETRIES, READ_TIMEOUT))
  {
    Serial.println("timeout");
    return "to";
  }
  else
  {
    char* data = (char*)&incoming_packet[1];
    if(data[0] == 'e' && data[1] == 'r' && data[2] == 'r')
    {  
      Serial.println("err");
    }
    else
    {
      Serial.println("noerr");
    }
    return data;
  }
}


char* get_info(unsigned int ss)
{
  static char info_buf[MAX_INFO_LEN];
  
  if(!exchange(ss, (char*)"f", 1, NUM_RETRIES, MSG_TIMEOUT))
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
  
  if(!exchange( ss, "e", 1, NUM_RETRIES, ECHO_TIMEOUT))
  {
    return false;
  }
  else if( incoming_packet[1] == 'e' )
  {
    return true;
  }

  return false;
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


bool write_chan(unsigned int ss, unsigned char chan, char* msg)
{
  int msglen = strlen( msg );
  char com[ 3 + msglen ];
  com [0] = 'W';
  com [1] = chan;
  com [2] = (msglen > MAX_PACKET_LENGTH - 2 )? MAX_PACKET_LENGTH - 2 : msglen;
  
  memcpy(&com[3], msg, com[2] );
  
  if(!exchange(ss, com, com[2], NUM_RETRIES, MSG_TIMEOUT))
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
