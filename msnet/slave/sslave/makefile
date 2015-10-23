main: 
	
mktemp: main.c analog.c analog.h slave.h spi_slave.c spi_slave.h ttemp_slave.c
	avr-gcc -Os -D_SLAVE_TTEMP_ -mmcu=attiny84 \
	-DF_CPU=8000000 -o main main.c analog.c spi_slave.c

temp: mktemp
	avr-objcopy -O ihex -R .eeprom main main.bin
	avrdude -p t84 -c usbtiny -U flash:w:main.bin


mkdummy1: main.c analog.c analog.h slave.h spi_slave.c spi_slave.h dummy_slave1.c
	avr-gcc -Os -D_SLAVE_DUMMY1_ -mmcu=attiny84 \
	-DF_CPU=8000000 -o main main.c analog.c spi_slave.c
	
mkdummy2: main.c analog.c analog.h slave.h spi_slave.c spi_slave.h dummy_slave2.c
	avr-gcc -Os -D_SLAVE_DUMMY2_ -mmcu=attiny84 \
	-DF_CPU=8000000 -o main main.c analog.c spi_slave.c
	
mkdummy3: main.c analog.c analog.h slave.h spi_slave.c spi_slave.h dummy_slave3.c
	avr-gcc -Os -D_SLAVE_DUMMY3_ -mmcu=attiny84 \
	-DF_CPU=8000000 -o main main.c analog.c spi_slave.c

mkdummy4: main.c analog.c analog.h slave.h spi_slave.c spi_slave.h dummy_slave4.c
	avr-gcc -Os -D_SLAVE_DUMMY4_ -mmcu=attiny84 \
	-DF_CPU=8000000 -o main main.c analog.c spi_slave.c
	
dummy1: mkdummy1
	avr-objcopy -O ihex -R .eeprom main main.bin
	avrdude -p t84 -c usbtiny -U flash:w:main.bin
	
dummy2: mkdummy2
	avr-objcopy -O ihex -R .eeprom main main.bin
	avrdude -p t84 -c usbtiny -U flash:w:main.bin
	
dummy3: mkdummy3
	avr-objcopy -O ihex -R .eeprom main main.bin
	avrdude -p t84 -c usbtiny -U flash:w:main.bin
	
dummy4: mkdummy4
	avr-objcopy -O ihex -R .eeprom main main.bin
	avrdude -p t84 -c usbtiny -U flash:w:main.bin
