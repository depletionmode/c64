#include "SRAM2.h"

/*
writestream1: Setup SRAM 1 in sequential write mode starting from the passed address.
              Bytes can then be written one byte at a time using RWdata(byte data).
			  Each byte is stored in the next location and it wraps around 32767.

writestream2: Setup SRAM 2 in sequential write mode starting from the passed address.
              Bytes can then be written one byte at a time using RWdata(byte data).
			  Each byte is stored in the next location and it wraps around 32767.

readstream1: Setup SRAM 1 in sequential read mode starting from the passed address.
             Bytes can then be read one byte at a time using  byte RWdata(0).The passed data is irrelavent.
			 Each byte is read from the next location and it wraps around 32767.

readstream2: Setup SRAM 2 in sequential read mode starting from the passed address.
             Bytes can then be read one byte at a time using  byte RWdata(0).The passed data is irrelavent.
			 Each byte is read from the next location and it wraps around 32767.

RWdata:      Write or read a byte at any time from the selected SRAM (1 or 2 may be selected).
             If the writesteam is open the passed byte will be written to the current address.
			 If the readstream is open the byte from the current address will be returned.

closeRWstream: Use to close the  open read or write stream for both SRAM 1 and 2.
               Dont need when changing between read/write.
			   Close before using SPI somewhere else.

digital pin 13    SCK
digital pin 12    MISO
digital pin 11    MOSI
digital pin 10    SS for SRAM 1
digital pin 9     SS for SRAM 2
*/

//#include "Arduino.h"

SRAM2::SRAM2()  //constructor
{
setupDDRB;
setupSPI;

deselectSS1;  //deselect if selected
deselectSS2;  //deselect if selected

selectSS1; //setup SRAM1
RWdata(0x05);//read status register
int Sreg = RWdata(0xff);//get status value
deselectSS1;

if(Sreg != 0x41) //are we in sequential mode
{
selectSS1;
RWdata(0x01); //write to status reg
RWdata(0x41);  //set sequencial  mode
deselectSS1;
}//end of set sequential mode for SRAM1

selectSS2; //setup SRAM2
RWdata(0x05);//read status register
Sreg = RWdata(0xff);//get status value
deselectSS2;

if(Sreg != 0x41) //are we in sequential mode
{
selectSS2;
RWdata(0x01); //write to status reg
RWdata(0x41);  //set sequencial  mode
deselectSS2;
}//end of set sequential mode for SRAM2
}//end of constructor

void SRAM2::writestream1(int address)
{
deselectSS1;  //deselect if still selected
deselectSS2;  //deselect if still selected
selectSS1; //select now
RWdata(0x02);//write to address
RWdata(address >> 8);//msb address
RWdata(address);//lsb address  
}//end of write stream SRAM 1

void SRAM2::writestream2(int address)
{
deselectSS1;  //deselect if still selected
deselectSS2;  //deselect if still selected
selectSS2; //select now
RWdata(0x02);//write to address
RWdata(address >> 8);//msb address
RWdata(address);//lsb address  
}//end of write stream SRAM 2

void SRAM2::readstream1(int address)
{
deselectSS1;  //deselect if still selected
deselectSS2;  //deselect if still selected
selectSS1; //select now
RWdata(0x03);//read from address
RWdata(address >> 8);//read from address
RWdata(address); 
}//end of read stream SRAM1

void SRAM2::readstream2(int address)
{
deselectSS2;  //deselect if still selected
deselectSS1;  //deselect if still selected
selectSS2; //select now
RWdata(0x03);//read from address
RWdata(address >> 8);//read from address
RWdata(address); 
}//end of read stream SRAM2

void SRAM2::closeRWstream(void)
{
deselectSS1;  //deselect
deselectSS2;  //deselect 
}//end of close RW stream

byte SRAM2::RWdata(byte data)
{
 SPDR = data;
  while (!(SPSR & _BV(SPIF)))
    ;
  return SPDR;
}//end of RWdata

SRAM2 SRAM;
