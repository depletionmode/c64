#pragma once
/* shamelessly stolen from somewhere */
#include <Arduino.h>

#define setupSPI SPCR = 0x50 //Master mode, MSB first, SCK phase low, SCK idle low, clock/4
#define setupDDRB DDRB |= 0x2e  //set  SCK(13) MOSI(11) and SS1 , SS2 as outputs 
#define selectSS1 PORTB &= ~0x04  //set the SS to 0 to select SRAM 1 
#define deselectSS1 PORTB |= 0x04  //set the SS to 1 to deselect SRAM 1 
#define selectSS2 PORTB &= ~0x02  //set the SS to 0 to select SRAM 2 
#define deselectSS2 PORTB |= 0x02  //set the SS to 1 to deselect SRAM 2 

class SRAM2
{
public:
	SRAM2();  //the constructor
	void writestream1(int address);
	void readstream1(int address);
	void writestream2(int address);
	void readstream2(int address);
	void closeRWstream(void);
	byte RWdata(byte data);
};

extern SRAM2 SRAM;

