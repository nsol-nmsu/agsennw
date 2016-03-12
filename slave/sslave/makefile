PROGRAMMER=usbtiny
OPTIMIZE=Os

hydra:  SLAVE = hydra_slave.c
intemp: SLAVE = intemp_slave.c
extemp: SLAVE = extemp_slave.c
hum:    SLAVE = hum_slave.c
adc:    SLAVE = adc_slave.c
dummy:  SLAVE = dummy_slave.c
wind:	SLAVE = wind_sensor.c

hydra:  F_CPU = 8000000
intemp: F_CPU = 8000000
extemp: F_CPU = 8000000
hum:    F_CPU = 8000000
adc:    F_CPU = 8000000
dummy:  F_CPU = 8000000
wind:	F_CPU = 1000000

hydra:  FUSES = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
intemp: FUSES = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
extemp: FUSES = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
hum:    FUSES = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
adc:    FUSES = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
dummy:  FUSES = -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
wind:	FUSES = -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m 

deploy: main.hex
	avrdude -u -p t84 -c $(PROGRAMMER) -U flash:w:main.hex \
	 $(FUSES)

	make clean

main.hex: main.elf
	avr-objcopy -O ihex -R .eeprom main.elf main.hex


main.elf: main.c analog.c analog.h slave.h spi_slave.c spi_slave.h sdi.h $(SLAVE)
	avr-gcc -Wall -$(OPTIMIZE) -mmcu=attiny84a \
	-DF_CPU=$(F_CPU) -o main.elf main.c analog.c spi_slave.c $(SLAVE)

clean:
	-rm *.elf *.hex

hydra:  deploy hydra_slave.c
intemp: deploy intemp_slave.c
extemp: deploy extemp_slave.c
hum:   deploy hum_slave.c
adc:   deploy adc_slave.c
dummy: deploy dummy_slave.c
wind:  deploy wind_sensor.c


