#include "MMU.h"
#include "SRAM2.h"
#include "CIA.h"
#include "VIC.h"

/*
==========
ROM LAYOUT
==========
$0000 - Test code	<- Gets mapped into Kernal address space by MMU in DEBUG mode
$1000 - Kernal
$3000 - Basic ROM
$5000 - Char ROM
*/

/*
===========
C64 MEM MAP
===========
$FFFF = 65535 ┌───────────────┬───────────────┐
              │---------------│|||||||||||||||│     ||| = is reading with PEEK
              │---------------│|||||||||||||||│     --- = is writing with POKE
              │---------------│|||||||||||||||│     +++ = read + write
              │---------------│||| KERNAL ||||│     other = not useable in BASIC 
              │---------------│|||||||||||||||│
              │---------------│|||||||||||||||│
              │---------------│|||||||||||||||│
$E000 = 57344 ├───────────────┼───────────────┼───────────────┐
              │               │               │+++++++++++++++│
              │               │     CHAR      │+++++ I/O +++++│
              │               │               │+++++++++++++++│
$D000 = 53248 ├───────────────┼───────────────┴───────────────┘
              │+++++++++++++++│
              │+++++++++++++++│
              │+++++++++++++++│
$C000 = 49152 ├───────────────┼───────────────┐
              │---------------│|||||||||||||||│
              │---------------│|||||||||||||||│
              │---------------│||| BASIC- ||||│
              │---------------│|||  ROM   ||||│
              │---------------│|||||||||||||||│
              │---------------│|||||||||||||||│
              │---------------│|||||||||||||||│
$A000 = 40960 ├───────────────┼───────────────┘
              │+++++++++++++++│
              │+++ BASIC- ++++│
              │+++  RAM   ++++│ \
              .               .  - Detail informations Memory (BASIC)            
              .               . /            
              .               .             
              │+++ BASIC- ++++│           
              │+++  RAM   ++++│
$0800 = 2048  │+++++++++++++++│-┐ 
$0400 = 1024  │+++++++++++++++│-┘Screen RAM
$0000
*/

#define __log(s)
//#define __log(s) Serial.write(s)

void MMU::init()
{
	CIA::init();

	// todo: use on-chip flash for registers in order to speed up
	writeRam(0x0000, 0x2f);	// 6510 data direction register
	writeRam(0x0001, 0x37);	// 6510 chip I/O port
}

void MMU::writeRam(unsigned int addr, byte val) {
	//e2_write(_ram_dev, addr, val);
	if (addr < 0x8000) SRAM.writestream1(addr);
	else SRAM.writestream2(addr - 0x8000);
	SRAM.RWdata(val);
}

byte MMU::readRam(unsigned int addr) {
	//return e2_read(_ram_dev, addr);
	if (addr < 0x8000) SRAM.readstream1(addr);
	else SRAM.readstream2(addr - 0x8000);
	return SRAM.RWdata(0xff);
}

void MMU::write(unsigned int addr, unsigned char val)
{
	// Write value to underlying memory addr regardless of mapping
	writeRam(addr, val);
	
	

	switch (addr >> 8) {
	case 0xd0:
	case 0xd1:
	case 0xd2:
	case 0xd3:
		VIC::poke(addr, val);
		break;
	case 0xdc:
		//CIA::write(CIA::CIA1, addr & 0xff, val);
		break;
	case 0xdd:
		//CIA::write(CIA::CIA2, addr & 0xff, val);
		break;
	case 0xdf:
		if (addr >> 8 == 0xff) Serial.write(val);
		break;
	default:
		break;
	}
	/*
	Serial.print("MMU::write(): ");
	Serial.print(addr, HEX);
	Serial.print(":");
	Serial.println(val, HEX);*/
}

char MMU::read(unsigned int addr)
{
	char val = 0;
	
	//Serial.print("MMU::read(): ");
	//Serial.println(addr, HEX);

	switch (addr >> 12)
	{
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
	case 0x8:
	case 0x9:
		if (addr == 0x0000) {
			__log("I/O Port DDR\n");
		} else if (addr == 0x0001) {
			__log("I/O Port DR\n");
		} else {
			__log("RAM BANK 1\n");
		}
		val = readRam(addr);
		break;
	case 0xa:
	case 0xb:
			__log("BASIC ROM\n");
			val = readRom(addr - 0x8000);
	case 0xc:
			__log("RAM BANK 2\n");
		break;
	case 0xd:
		if (!(readRam(0x0001) & 0x4)) {
			__log("Char ROM\n");
			val = readRom(addr - 0x8000);
		} else {
			__log("I/O\n");
			switch (addr >> 8)
			{
			case 0xd0:
			case 0xd1:
			case 0xd2:
			case 0xd3:
				__log("VIC Registers\n");
				val = VIC::peek(addr);
				break;
			case 0xd4:
			case 0xd5:
			case 0xd6:
			case 0xd7:
				__log("SID Registers\n");
				break;
			case 0xd8:
			case 0xd9:
			case 0xda:
			case 0xdb:
				__log("Color RAM\n");
				break;
			case 0xdc:
				//CIA::read(CIA::CIA1, addr & 0xff);
				__log("CIA 1\n");
				break;
			case 0xdd:
				//CIA::read(CIA::CIA2, addr & 0xff);
				__log("CIA 2\n");
				break;
			case 0xde:
				__log("I/O 1\n");
				break;
			case 0xdf:
				__log("I/O 2\n");
				// if read from 0xdfff, return serial port data
				if (addr == 0xdfff) {
					if (Serial.available()) val = Serial.read() & 0xff;
					else val = 0;
				}
				break;
			default:
				__log("MMU error: Undef mem location in I/O, Char ROM!\n");
			}
			
			val = readRam(addr);
		}
		break;
	case 0xe:
	case 0xf:
			__log("KERNAL ROM\n");
			// DEBUG reads from $0000 instead of from $1000 which is where the kernel is...
			val = readRom(addr - 0xe000);
			//val = readRom(addr - 0xd000);
		break;
	default:
		__log("MMU error: Undef mem location!\n");
	}

	return val;
}

void MMU::writeRom(unsigned int addr, byte val)
{
	// slow
	/*static bool init = 0;
	if (!init) {
		Wire.begin();
		init++;
	}*/

	Wire.beginTransmission(ROM_DEVICE);
	Wire.write(addr >> 8);
	Wire.write(addr & 0xff);
	Wire.write(val);
	Wire.endTransmission();
	delay(5);
}

byte MMU::readRom(unsigned int addr)
{
	// slow
	/*static bool init = 0;
	if (!init) {
		Wire.begin();
		init++;
	}*/

	Wire.beginTransmission(ROM_DEVICE);
	Wire.write(addr >> 8);
	Wire.write(addr & 0xff);
	Wire.endTransmission(false);
	Wire.requestFrom(ROM_DEVICE, 1);
	byte val = Wire.read() & 0xff;
	return val;
}

void MMU::burnRom(unsigned int addr, byte *data, int len)
{
	for (int i = 0; i < len; i++) {
		//digitalWrite(13, HIGH);
		writeRom(addr + i, data[i]);
		//digitalWrite(13, LOW);
	}
}

void MMU::setRom(unsigned int addr, byte val, long count)
{
	// this doesn't seem tw work?
	for (int i = 0; i < count; i++) {
		//digitalWrite(13, HIGH);
		writeRom(addr + i, val);
		//digitalWrite(13, LOW);
	}
}