#ifndef SPI_SLAVE_TRANSPORT_PROTOCOL
#define SPI_SLAVE_TRANSPORT_PROTOCOL

#include <avr/io.h>
#include <avr/interrupt.h>

/* USI port and pin definitions. Since this slave code will run on different CPUs, we
use preprocessor directives to grab the right ports and pins. A list of all avr definitions
can be found at http://www.nongnu.org/avr-libc/user-manual/io_8h_source.html
 */
#if defined (__AVR_ATtiny85__)
#  define USI_OUT_REG          PORTB  //!< USI port output register.
#  define USI_IN_REG          PINB  //!< USI port input register.
#  define USI_DIR_REG          DDRB  //!< USI port direction register.
#  define USI_CLOCK_PIN          PB2    //!< USI clock I/O pin.
#  define USI_DATAIN_PIN    PB0    //!< USI data input pin.
#  define USI_DATAOUT_PIN    PB1      //!< USI data output pin.
#  define SPI_SS_PIN            PB4     //!< SPI slave select pin
#  define TIMERMASK             TIMSK
#  define TIMERFLAG             TIFR
#  define PINCHANGE             PCMSK
#  define PINFLAG               PCIE
#  define TEMP_PIN             PB3
//#  define MUX_INPUT             3   //ADMUX setting lower 4 bits : 0011 corresponds to PB3 (ADC3)

#elif defined (__AVR_ATtiny84__)
#  define USI_OUT_REG          PORTA
#  define USI_IN_REG           PINA
#  define USI_DIR_REG          DDRA
#  define USI_CLOCK_PIN        PA4
#  define USI_DATAIN_PIN       PA6
#  define USI_DATAOUT_PIN      PA5
#  define SPI_SS_PIN           PA7
#  define TIMERMASK            TIMSK0
#  define TIMERFLAG            TIFR0
#  define PINCHANGE            PCMSK0
#  define PINFLAG              PCIE0
#  define TEMP_PIN          PA2
//#  define MUX_INPUT            2   //ADMUX setting lower 2 bits: 0010 corresponds to PA2 (ADC2)

#endif

#define MAX_PACKET_LENGTH 30


/*  Speed configuration:
 *  Bits per second = CPUSPEED / PRESCALER / (COMPAREVALUE+1) / 2.
 *  Maximum = CPUSPEED / 64.
 */
#define TC0_PRESCALER_VALUE 1  //!< Must be 1, 8, 64, 256 or 1024.
#define TC0_COMPARE_VALUE   31  //!< Must be 0 to 255. Minimum 31 with prescaler CLK/1.



/*  Prescaler value converted to bit settings.
 */
#if TC0_PRESCALER_VALUE == 1
  #define TC0_PS_SETTING (1<<CS00)
#elif TC0_PRESCALER_VALUE == 8
  #define TC0_PS_SETTING (1<<CS01)
#elif TC0_PRESCALER_VALUE == 64
  #define TC0_PS_SETTING (1<<CS01)|(1<<CS00)
#elif TC0_PRESCALER_VALUE == 256
  #define TC0_PS_SETTING (1<<CS02)
#elif TC0_PRESCALER_VALUE == 1024
  #define TC0_PS_SETTING (1<<CS02)|(1<<CS00)
#else
  #error Invalid T/C0 prescaler setting.
#endif


#define SPIMODE 1         /*!< \def The SPI mode to run in. Can be 0 or 1 for USI.*/

#define RECEIVE_CHAR '$'  /*!< \def The char to check for to indicate the start of a packet from the master*/
#define SEND_CHAR 0xFE    /*!< \def The char to check for to indicate that the master is waiting on bytes from the slave*/


/*! \brief  Data input register buffer.
 *
 *  Incoming bytes are stored in this byte until the next transfer is complete.
 *  This byte can be used the same way as the SPI data register in the native
 *  SPI module, which means that the byte must be read before the next transfer
 *  completes and overwrites the current value.
 */
extern unsigned char storedUSIDR;
extern unsigned char numReceived;


/*! \brief  Driver status bit structure.
 *
 *  This struct contains status flags for the driver.
 *  The flags have the same meaning as the corresponding status flags
 *  for the native SPI module. The flags should not be changed by the user.
 *  The driver takes care of updating the flags when required.
 */
struct usidriverStatus_t {
  unsigned char masterMode : 1;       //!< True if in master mode.
  unsigned char transferComplete : 1; //!< True when transfer completed.
  unsigned char writeCollision : 1;   //!< True if put attempted during transfer.
    unsigned char slaveSelected : 1;
    unsigned char receiving : 1;  //!< True if we should be in receiving state
    };




extern volatile struct usidriverStatus_t spiX_status; //!< The driver status bits.

//we declare 2 arrays to be reused for packet sending and receiving
extern unsigned char outgoing_packet[MAX_PACKET_LENGTH];
extern unsigned char incoming_packet[MAX_PACKET_LENGTH];


/*! \brief Enables the USI module.
 *
 * This function enables the USI module so the slave can put bits onto the SPI bus.
 * Used when the slave is selected.
 *
 */
void usi_enable();

/*! \brief Disables the USI module.
 *
 * This function disables the USI module so this slave cannot put bits onto the SPI bus.
 * Used when the slave is deselected to prevent collisions.
 *
 */
void usi_disable();


/*! \brief  Initialize USI as SPI slave.
 *
 *  This function sets up all pin directions and module configurations.
 *  Use this function initially or when changing from master to slave mode.
 *  Note that the stored USIDR value is cleared.
 *
 *  \param spi_mode  Required SPI mode, must be 0 or 1.
 */
void spiX_initslave(char spi_mode);

/*! \brief  Put one byte on bus.
 *
 *  Use this function like you would write to the SPDR register in the native SPI module.
 *  A byte will be prepared for the next transfer initiated by the master device.
 *  If a transfer is in progress, this function will set the write collision flag
 *  and return without altering the data registers.
 *
 *  \param val The byte value to put on the bus.
 *  \return  0 if a write collision occurred, 1 otherwise.
 */
char spiX_put(unsigned char val);

/*! \brief  Get one byte from bus.
 *
 *  This function only returns the previous stored USIDR value.
 *  The transfer complete flag is not checked. Use this function
 *  like you would read from the SPDR register in the native SPI module.
 *  spiX_wait() should be called before this function to ensure a new byte has been transferred.
 *  \return The byte received from the master.
 */
unsigned char spiX_get();

/*! \brief  Wait for transfer to complete.
 *
 *  This function waits until the transfer complete flag is set.
 *  Use this function like you would wait for the native SPI interrupt flag.
 *  NOTE: \a spiX_put() must be called before \a spiX_wait() in order to reset the transfer complete flag.
 */
void spiX_wait();

/*! \brief Sends bytes from \a outgoing_packet until end of packet. This function blocks.
 *
 *  This function sends bytes from \a outgoing_packet to the master.
 *  Number of bytes is contained in \a outgoing_packet[0] after \a prepare_packet() is run.
 *
 */
void send_packet();

/*! \brief Receives bytes into incoming_packet. This function blocks.
 *  This function waits for bytes from the master and reads in the number of bytes given by the master as the 1st byte of the packet.
 *  If MAX_PACKET_LENGTH is reached, function exits.
 *  Does not integrity check.
 *  \sa incoming_packet
 */
void receive_packet();

/*! \brief Create a packet for transferring
*
*  This function takes the payload and wraps it into a packet using our standard format:
*  LENGTH PAYLOAD  ENDOFMESSAGEDELIMITER CRC '#'
*  This method will compute the CRC of the payload.
*  Fills in outgoing_packet. If length is >MAX_PACKET_LENGTH-5, the outgoing_packet is
*  not filled in and 0 is returned.
*  NOTE: array is not null terminated, so it should not be treated like a string
*  /param payload The byte array to prepare
*  /param length The length of payload
*  /return 1 if preparation was successful, 0 otherwise
*  /sa outgoing_packet
*/
int prepare_packet(char* payload, int length);

/*! \brief Checks the crc of the incoming_packet for integrity
 *  Calculates the CRC-16 of incoming_packet and compares it to the CRC given in incoming_packet.
 *
 *  \return 1 if crc matches, 0 otherwise.
 *  \sa outgoing_packet
 */
int check_integrity();
int slave_selected();

#endif /* SPI_SLAVE_TRANSPORT_PROTOCOL */
