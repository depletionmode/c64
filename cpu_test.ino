/* 
 * 6502 test programs 
 *
 * In order to run these, MMU va $0e00 needs to point to ROM $0000
 *
 * Choose test by uncommenting relevant cpu.reset() below
 */

#include <Wire.h>

#include "SRAM2.h"
#include "MMU.h"
#include "CPU_6502.h"
#include "CIA.h"
#include "cpu_tests.h"

#if 1
CPU_6502 cpu;

void setup() {
  Serial.begin(9600);
  //pinMode(13, OUTPUT);
  //digitalWrite(13, HIGH);
  Serial.write("6502 tests\n");
  Wire.begin();
    
  /* write test programs to ROM starting @ $0000 */
  MMU::burnRom(0x0000, uart_echo, sizeof(uart_echo));
  MMU::burnRom(0x0100, test_adc_1, sizeof(test_adc_1));

  Serial.write("Executing test program...\n\n");  
  
  /* choose test program to run */
  cpu.reset(0xe000);	// uart echo
  //cpu.reset(0xe100);	// adc 1
}

unsigned long time;

void loop() {
	cpu.execIns();
}
#endif