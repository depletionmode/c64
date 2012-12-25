#pragma once
#include <Arduino.h>
#include <Wire.h>

#define ROM_DEVICE 0x50

class MMU
{
public:
	MMU() {};
	static void init();

	static void write(unsigned int addr, unsigned char val);
	static char read(unsigned int addr);

	static void burnRom(unsigned int addr, byte *data, int len);
	static void setRom(unsigned int addr, byte val, long count);

private:
	static void writeRam(unsigned int addr, byte val);
	static byte readRam(unsigned int addr);

	static void writeRom(unsigned int addr, byte val);
	static byte readRom(unsigned int addr);
};

