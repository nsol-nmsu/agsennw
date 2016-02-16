PROGRAMMER=usbtiny

deploy: main.hex
	avrdude -p t84 -c $(PROGRAMMER) -U flash:w:main.hex #-P /dev/ttyUSB1

main.hex: main.elf
	avr-objcopy -O ihex -R .eeprom main.elf main.hex


main.elf: main.c analog.c analog.h slave.h spi_slave.c spi_slave.h slave_defs.h sdi.h
	avr-gcc -Wall -Os -mmcu=attiny84 \
	-DF_CPU=8000000 -o main.elf main.c analog.c spi_slave.c
