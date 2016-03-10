#include "spi_master.h"
#include "Arduino.h"
#include <util/crc16.h>
#include <avr/delay.h>

unsigned char outgoing_packet[MAX_PACKET_LENGTH];
unsigned char incoming_packet[MAX_PACKET_LENGTH];

void SPI_setup()
{
  SPI.begin();
  
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setDataMode(SPI_MODE1);
}

int prepare_packet(char* payload, int length)
{
 if(length > MAX_PACKET_LENGTH-5) return 0;

 outgoing_packet[0] = length+5;
 
 int i;
 for(i = 0; i < length; i++)
 {
     outgoing_packet[i+1] = payload[i];
 }
 
 outgoing_packet[length+1] = 0;
 
 //CRC
 uint16_t crc = 0xffff;
 for(i = 0; i < length+1; i++)
 {
   crc = _crc16_update(crc, outgoing_packet[i]); 
 }
 //break CRC into two halves
 //take lower 8 bits first
 outgoing_packet[length+2] = crc & 0xff;
 //then the upper 8 bits
 outgoing_packet[length+3] = crc >> 8;
 
 outgoing_packet[length+4] = '#';
 
 return 1;
}


//method to check the integrity of incoming_packet that is received
int check_integrity()
{
 int length = (int) incoming_packet[0];
 //check if the packet length is the same as the length 
 //the packet says it is supposed to be
 /*
 int countedLength = 0;
 while((countedLength < 30) && (incoming_packet[countedLength] != '#'))
 {
   countedLength++;
 }
 if(countedLength+1 != length)
 {
   Serial.println("length mismatch"); 
   Serial.print(countedLength+1); 
   Serial.print(" != ");
   Serial.println(length);
 
   return 0; 
 }
 */
 if(length > MAX_PACKET_LENGTH) return 0;

 
 uint16_t crc = 0xffff;
 int i;
 for(i = 0; i < length-4; i++)
 {
   crc = _crc16_update(crc, incoming_packet[i]); 
 }
 
 uint16_t found_crc = incoming_packet[length-3] | (incoming_packet[length-2] <<8);
 
 if(found_crc == crc)  return 1;
 return 0;
}


//method to send a packet
int send_packet(int timeout)
{
  int packet_length = outgoing_packet[0];
  unsigned char buff;
  
  //send $ to indicate start of packet (send a few to make sure slave stays in receiving state)
  buff = writeSlave((unsigned char)'$');
  
  unsigned long start_time = millis();
  while(buff != (unsigned char)'!'){
    if(millis()-start_time > timeout){
      return 0;
    }
    delayMicroseconds(SPI_DELAY); 
    buff = writeSlave((unsigned char)'$');
  }
  delayMicroseconds(SPI_DELAY);
  int index = 0;
  while(index < MAX_PACKET_LENGTH && index < packet_length)
  {
    writeSlave((unsigned char)outgoing_packet[index]);
    index++;
    delayMicroseconds(SPI_DELAY);
  }
  
  if(index == MAX_PACKET_LENGTH){Serial.println("Error, sent max unsigned chars"); return 0;}
  //send '#'
  writeSlave((unsigned char)outgoing_packet[index]);
  
  return 1;
}


//method to receive a packet
int receive_packet(int timeout)
{
  delayMicroseconds(SPI_DELAY);
  unsigned char buff;
  unsigned long start_time = millis();
  //read unsigned chars until we get the start of packet char, $
  while((buff=writeSlave(0xFE)) != (unsigned char)'$'){
    
    if(millis()-start_time > timeout)
      return 0;
      
    delayMicroseconds(SPI_DELAY);
  }
  delayMicroseconds(SPI_DELAY);
  start_time = millis();
  while((buff=writeSlave(0xFE)) == (unsigned char)'$')
  {
    if(millis()-start_time > timeout)
      return 0;
    delayMicroseconds(SPI_DELAY); 
  }
  //indicate to slave we got start of packet
  int index = 0;
  
  //get first unsigned char
  incoming_packet[index] = buff;  //this byte contains the length of the packet; we will get bytes until we reach that length or MAX_PACKET_LENGTH
  index++;
  delayMicroseconds(SPI_DELAY);
  while(index < MAX_PACKET_LENGTH)
  {
    if(index == incoming_packet[0]) break;
    incoming_packet[index] = writeSlave(0xFE);
    //if(incoming_packet[index] == '#') break;
    index++;
    delayMicroseconds(SPI_DELAY);
  }
  //write some '$' to get slave to switch state
  writeSlave('$');
  writeSlave('$');
  return 1;
}

unsigned int writeSlave(byte thisValue)
{
  unsigned int result = SPI.transfer(thisValue);
  
  return result;
}

/*
*Does a full message exchange with slave (i.e. a send and receive). received message is stored in incoming_packet
*Convenience function to perform message send, receive, and CRC check (with optional retry)
*with a single slave (does slave selection internally)
*returns 1 if function succeded, 0 if num_retries became 0 or timeout exceeded during send or receive.
*/
int exchange(int slave, char* command, int command_length, int num_retries, int timeout)
{
  //Select slave
  digitalWrite(slave,LOW);
  //wait a little to allow slave to be ready
  delay(SLAVE_SWITCH_TIME);
  
  //prepare command
  prepare_packet(command, command_length);
  
  //send command
  if(!send_packet(timeout)){
    digitalWrite(slave, HIGH);
    return 0;
  }
  
  //wait if we request an EEPROM write
  if(outgoing_packet[1] == 'q'){
    delay(1000);
  }
  
  //receive command
  if(!receive_packet(timeout)){
    digitalWrite(slave, HIGH);
    return 0;
  }
  //if retry is set, loop until CRC checks out
  while(num_retries)
  {
    if(check_integrity())
    {
      //try again if slave responded with 'err'
      if(incoming_packet[0] >= 8 && incoming_packet[1] == 'e' && incoming_packet[2] == 'r' && incoming_packet[3] == 'r')
      {
        if(!send_packet(timeout)){
          digitalWrite(slave, HIGH);
          return 0;
        }
         //wait if we request an EEPROM write
        if(outgoing_packet[1] == 'q'){
          delay(1000);
        }
 
        if(!receive_packet(timeout)){
          return 0;
          digitalWrite(slave, HIGH);
        }
        num_retries--;
        continue;
      }
      break;
    }
    if(!send_packet(timeout)){
      digitalWrite(slave, HIGH);
      return 0;
    }
    if(!receive_packet(timeout)){
      digitalWrite(slave, HIGH);
      return 0;
    }
    num_retries--;
  }
  
  //deselect slave
  digitalWrite(slave, HIGH);
  
  if(num_retries)  return 1; //we were successful.
  return 0; //ran out of retries
  
}
