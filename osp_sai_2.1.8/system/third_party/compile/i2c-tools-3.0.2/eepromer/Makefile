#eepromer  Makefile

CFLAGS = -O2 -I../include -Wall

CC=$(CROSS_COMPILE)gcc

all:  eeprom 
#all:  eepromer eeprom eeprog

eepromer:  eepromer.o

eeprom:  eeprom.c
	$(CC) -Wall -o $@ $^ $(CFLAGS)

eeprog: eeprog.o 24cXX.o


clean:
	rm -rf *~ *.o eepromer eeprom eeprog
