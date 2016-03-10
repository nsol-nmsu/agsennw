PROGRAMMER=usbtiny
OPTIMIZE=Os

hydra:  SLAVE = hydra_slave.c
intemp: SLAVE = intemp_slave.c
extemp: SLAVE = extemp_slave.c
hum:    SLAVE = hum_slave.c
adc:    SLAVE = adc_slave.c
dummy:  SLAVE = dummy_slave.c

deploy: main.hex
	avrdude -u -p t84 -c $(PROGRAMMER) -U flash:w:main.hex \
	 -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
main.hex: main.elf
	avr-objcopy -O ihex -R .eeprom main.elf main.hex


main.elf: main.c analog.c analog.h slave.h spi_slave.c spi_slave.h sdi.h $(SLAVE)
	avr-gcc -Wall -$(OPTIMIZE) -mmcu=attiny84a \
	-DF_CPU=8000000 -o main.elf main.c analog.c spi_slave.c $(SLAVE)

hydra:  deploy hydra_slave.c
intemp: deploy intemp_slave.c
extemp: deploy extemp_slave.c
hum:   deploy hum_slave.c
adc:   deploy adc_slave.c
dummy: deploy dummy_slave.c
