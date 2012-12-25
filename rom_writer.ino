#include <Wire.h>

#include "MMU.h"

#define BLOCK_SIZE 16
#if 0
void setup() {
	// init
	Wire.begin();
	Serial.begin(9600);
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);

	// read address from serial
	unsigned int addr = Serial.read() << 24 |
						Serial.read() << 16 |
						Serial.read() << 8 |
						Serial.read();

	// read length from serial
	unsigned int len =  Serial.read() << 24 |
						Serial.read() << 16 |
						Serial.read() << 8 |
						Serial.read();
	
	// read data from serial
	byte buf[BLOCK_SIZE];
	int n = 0;
	while (n < len) {
		int m = Serial.readBytes((char *)buf, len < BLOCK_SIZE ? len : BLOCK_SIZE);
		/*Serial.print("Writing 0x");
		Serial.print(m, HEX);
		Serial.print(" bytes to $");
		Serial.print(addr + n);
		Serial.print(" [");
		Serial.print((int)(n + m / len));
		Serial.println("%]");*/
		MMU::burnRom(addr + n, buf, m);
		n += m;
	}
}

void loop() {}
#endif