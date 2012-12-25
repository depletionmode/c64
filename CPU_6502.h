#pragma once

#include <Arduino.h>

class CPU_6502
{
public:
	CPU_6502();

	void execIns();
	void reset(unsigned int pc);

private:
	unsigned int _regPC;
	byte _regS;
	byte _regP;
	byte _regA;
	byte _regX;
	byte _regY;

	byte _rom_dev;
	byte _ram_dev;

	void jump_branch(int offset);
	void ins_ADC(byte value);
	void stack_push(byte val);
	byte stack_pop();
	void ins_AND(byte val);
	void ins_ASL(byte *val);
	void ins_BIT(byte val);
	void ins_CMP(byte reg, byte val);
	void ins_DEC(int addr);
	void ins_EOR(byte val);
	void ins_INC(int addr);
	void ins_LD_(byte *reg, int addr);
	void ins_LSR(byte *val);
	void ins_ORA(byte val);
	void ins_ROL(byte *val);
	void ins_ROR(byte *val);
	void ins_SBC(byte val);

	byte fetch();
	
	/* P (Status) Register */
#define C 1
#define Z 2
#define I 4
#define D 8
#define B 0x10
#define V 0x40
#define N 0x80

#define SET_C _regP |= C
#define CLR_C _regP &= ~C
#define IS_C (_regP & C > 0)
#define SET_Z _regP |= Z
#define CLR_Z _regP &= ~Z
#define IS_Z (_regP & Z > 0)
#define SET_I _regP |= I
#define CLR_I _regP &= ~I
#define IS_I (_regP & I > 0)
#define SET_D _regP |= D
#define CLR_D _regP &= ~D
#define IS_D (_regP & D > 0)
#define SET_B _regP |= B
#define CLR_B _regP &= ~B
#define IS_B (_regP & B > 0)
#define SET_V _regP |= V
#define CLR_V _regP &= ~V
#define IS_V (_regP & V > 0)
#define SET_N _regP |= N
#define CLR_N _regP &= ~N
#define IS_N (_regP & N > 0)

	bool lineReady;
	bool lineIRQ;
	bool lineNMI;

	int _state;
};

