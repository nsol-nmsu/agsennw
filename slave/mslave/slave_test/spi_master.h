#ifndef INC_SPI_MASTER_TRANSPORT_PROTOCOL
#define INC_SPI_MASTER_TRANSPORT_PROTOCOL

#include <SPI.h>

#define MAX_PACKET_LENGTH  60   /*!< \def The maximum packet length for outgoing and incoming packets*/
#define SPI_DELAY          300  /*!< \def The time in microseconds between sending bytes. Should not be lower than 300*/
#define SLAVE_SWITCH_TIME  2   /*!< \def The time in milliseconds between switching slaves. Affects wait time after setting slave select in function exchange*/

extern unsigned char outgoing_packet[MAX_PACKET_LENGTH];
extern unsigned char incoming_packet[MAX_PACKET_LENGTH];

/*! \brief Sets up this messaging driver.
 *
 *  Initializes the SPI driver. Uses SPI mode 1 with a clock division of 32.
 *  Should be called before using this driver.
 */
void SPI_setup();

/*! \brief Prepares a packet in outgoing_packet with \a length bytes of \a payload.
 *
 *  Creates and stores the envelope and the \a payload into outgoing_packet. \a length must not be more than MAX_PACKET_LENGTH - 5.
 *  \param payload  A pointer to the bytes to store in outgoing_packet
 *  \param length   The number of bytes to read from payload
 *  \return 0 if preparation was unsuccessful (due to length > MAX_PACKET_LENGTH - 5), 1 otherwise.
 */
int prepare_packet(char* payload, int length);

/*! \brief Checks the integrity of incoming_packet.
 *
 *  Computes the CRC-16 of incoming_packet and compares it to the given CRC-16 in incoming_packet.
 *  \return 0 if CRC doesn't match, 1 otherwise.
 */
int check_integrity();

/*! \brief Sends outgoing_packet over SPI.
 *
 *  Sends the contents of outgoing_packet over SPI. Will quit early if timeout is reached while waiting for slave to send ready response.
 *  Slave must be selected before calling this function.
 *
 *  \param timeout  The time in milliseconds to wait for ready response from slave.
 *  \return 0 if timeout occurred, 1 otherwise.
 */
int send_packet(int timeout);

/*! \brief Receives a packet into incoming_packet over SPI.
 *
 *  Receives a packet over SPI and stores it into incoming_packet. Will quit early if timeout is reached while waiting for start of packet from slave.
 *  Slave must be selected before calling this function.
 *
 *  \param timeout  The time in milliseconds to wait for ready response from slave.
 *  \return 0 if timeout occurred, 1 otherwise.
 */
int receive_packet(int timeout);

/*! \brief Sends a byte over SPI.
 *
 *  Sends a byte over SPI and returns the value from the slave.
 *  Slave must be selected before calling this function.
 *
 *  \param thisValue  The byte to send to the slave.
 *  \return The byte received from the slave during exchange.
 */
unsigned int writeSlave(unsigned char thisValue);

/*! \brief Sends outgoing_packet and receives a packet into incoming_packet over SPI with a given slave.
 *
 *  This function is a simple interface for exchanging packets with a particular slave. Will select the slave, prepare the packet, send the packet, and receive a packet.
 *  After receiving, the function checks for integrity. If the integrity doesn't match, the function will resend the command num_retries many times.
 *
 *  \param slave  The slave to select for the exchange
 *  \param command The command/payload to send to the slave
 *  \param command_length  The length of command
 *  \param num_retries  The number of times to retry sending the message.
 *  \param timeout  The time in milliseconds to wait for ready response from slave.
 *  \return 0 if num_retries is exceeded (indicating failure to send/receive a proper packet), 1 otherwise.
 *  \sa prepare_packet(), send_packet(), receive_packet(), check_integrity()
 */
int exchange(int slave, char* command, int command_length, int num_retries, int timeout);

#endif /* INC_SPI_MASTER_MESSAGE_PROTOCOL */
