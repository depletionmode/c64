#include "CPU_6502.h"
#include "SRAM2.h"
#include "MMU.h"

#include <Arduino.h>

#define _dbg(X)// Serial.println(X)

// TODO: refactor
void CPU_6502::jump_branch(int offset) {
  if( offset > 0x7f )
    _regPC = (_regPC - (0x100 - offset));
  else
    _regPC += offset;
}

void CPU_6502::stack_push(byte val) {
  if (_regS >= 0) {
    _regS--;
    MMU::write(_regS + 0x100, val);
  } 
}

byte CPU_6502::stack_pop() {
  if (_regS < 0x100) {
    byte val = MMU::read(_regS++ + 0x100);
    return(val);
  }
  
  return(0);
}

void CPU_6502::ins_ADC(byte val) {
	byte tmp = _regA + val + IS_C ? 1 : 0;
	if (tmp < _regA) SET_C;
	
	if ((_regA ^ val) & 0x80) CLR_V; else SET_V;
	_regA = tmp;

	if (_regA < tmp) SET_C;
	if (_regA) CLR_Z; else SET_Z;
	if (_regA & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_AND(byte val) {
	_regA &= val;
	if (_regA) CLR_Z; else SET_Z;
	if (_regA & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_ASL(byte *val)
{
	if (*val & 0x80) SET_C; else CLR_C;
	*val <<= 1; 
	if (*val) CLR_Z; else SET_Z;
	if (*val & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_ROL(byte *val)
{
	byte carry = IS_C ? 1 : 0;
	if (*val & 0x80) SET_C; else CLR_C;
	*val <<= 1;
	*val |= carry;
	if (*val) CLR_Z; else SET_Z;
	if (*val & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_ROR(byte *val)
{
	byte carry = IS_C ? 1 : 0;
	if (*val & 1) SET_C; else CLR_C;
	*val >>= 1;
	*val |= carry << 7;
	if (*val) CLR_Z; else SET_Z;
	if (*val & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_LSR(byte *val)
{
	if (*val & 1) SET_C; else CLR_C;
	*val >>= 1; 
	if (*val) CLR_Z; else SET_Z;
	if (*val & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_BIT(byte val) {
	if (val & _regA) CLR_Z; else SET_Z;
	if (val & N) SET_N; else CLR_N;
	if (val & V) SET_V; else CLR_V;
}

void CPU_6502::ins_CMP(byte reg, byte val) {
  if (reg >= val) SET_C; else CLR_C;
  if (reg == val) SET_Z; else CLR_Z;
  if (val & 0x80) SET_N; else CLR_N; // todo - check this
}

void CPU_6502::ins_DEC(int addr) {
	byte val = MMU::read(addr) - 1;
	MMU::write(addr, val);
	if (val) CLR_Z; else SET_Z;
	if (val & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_EOR(byte val) {
  _regA ^= val;
  if (_regA) CLR_Z; else SET_Z;
  if (_regA & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_INC(int addr) {
	byte val = MMU::read(addr) + 1;
	MMU::write(addr, val);
	if (val) CLR_Z; else SET_Z;
	if (val & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_LD_(byte *reg, int addr) {
	*reg = MMU::read(addr);
	if (*reg) CLR_Z; else SET_Z;
	if (*reg & 0x80) SET_N; else CLR_N;
}

void CPU_6502::ins_ORA(byte val) {
	_regA |= val;
	if (_regA) CLR_Z; else SET_Z;
	if (_regA & 0x80) SET_N; else CLR_N;
}

// TODO: refactor
void CPU_6502::ins_SBC(byte val) {
	ins_ADC(val ^ 0xff);
	/*
  int vflag,tmp,w;
  if( (_regA ^ value ) & 0x80 )
    vflag = 1;
  else
    vflag = 0;

  if( _regP & 8 ) {
    tmp = 0xf + (_regA & 0xf) - (value & 0xf) + (_regP&1);
    if( tmp < 0x10 ) {
      w = 0;
      tmp -= 6;
    } else {
      w = 0x10;
      tmp -= 0x10;
    }
    w += 0xf0 + (_regA & 0xf0) - (value & 0xf0);
    if( w < 0x100 ) {
      _regP &= 0xfe;
      if( (_regP&0xbf) && w<0x80) _regP&=0xbf;
      w -= 0x60;
    } else {
      _regP |= 1;
      if( (_regP&0xbf) && w>=0x180) _regP&=0xbf;
    }
    w += tmp;
  } else {
    w = 0xff + _regA - value + (_regP&1);
    if( w<0x100 ) {
      _regP &= 0xfe;
      if( (_regP&0xbf) && w<0x80 ) _regP&=0xbf;
    } else {
      _regP |= 1;
      if( (_regP&0xbf) && w>= 0x180) _regP&=0xbf;
    }
  }
  _regA = w & 0xff;
  if( _regA ) CLR_Z; else SET_Z;
  if (_regA & 0x80) SET_N; else CLR_N;*/
}

byte CPU_6502::fetch() {
    byte val = MMU::read(_regPC);
    _regPC += 1;
  return(val);
  
}

void CPU_6502::reset(unsigned int pc) {
	MMU::init();

  _regA = _regX = _regY = 0;
  _regS = 0x100;
  _regP = 0x20;
  _regPC = pc;

  _state = 0;
  _cycleCntr = 0;
}

void CPU_6502::execIns() {
  unsigned int addr, tmp;
  byte val, zp, off;
  byte opcode = fetch();
 
  switch(opcode) {
    /*** ADC ***/
    case 0x61:    _dbg("ADC INDX");
      zp = fetch() + _regX;
      addr = MMU::read(zp) + (MMU::read(zp + 1) << 8);
      ins_ADC(MMU::read(addr));
      break;
    case 0x65:    _dbg("ADC ZP");
		zp = fetch();
		ins_ADC(MMU::read(zp));
		break;
    case 0x69:    _dbg("ADC IMM");
      ins_ADC(fetch());
      break;
    case 0x6d:    _dbg("ADC ABS");
      addr = fetch() | fetch() << 8;
      ins_ADC(MMU::read(addr));
      break;
    case 0x71:    _dbg("ADC INDY");
      zp = fetch();
      addr = (MMU::read(zp) | MMU::read(zp + 1) << 8) + _regY;
      ins_ADC(MMU::read(addr));
      break;
    case 0x75:    _dbg("ADC ZPX");
		zp = fetch() + _regX;
		val = MMU::read(zp);
		ins_ADC(val);
		break;
    case 0x79:    _dbg("ADC ABSY");
      addr = (fetch() | fetch() << 8) + _regY;
      ins_ADC(MMU::read(addr));
      break;
    case 0x7d:    _dbg("ADC ABSX");
      addr = (fetch() | fetch() << 8) + _regX;
      ins_ADC(MMU::read(addr));
      break;
    /*** ADC END ***/

    /*** AND ***/
    case 0x21:    _dbg("AND INDX");
      zp = fetch() + _regX;
      addr = MMU::read(zp) + (MMU::read(zp + 1) << 8);
      ins_AND(MMU::read(addr));
      break;
    case 0x25:    _dbg("AND ZP");
      ins_AND(MMU::read(fetch()));
      break;
    case 0x29:    _dbg("AND IMM");
      ins_AND(fetch());
      break;
    case 0x2d:    _dbg("AND ABS");
      addr = fetch() | fetch() << 8;
      ins_AND(MMU::read(addr));
      break;
    case 0x31:    _dbg("AND INDY");
      zp = fetch();
      addr = MMU::read(zp) + (MMU::read(zp + 1) << 8) + _regY;
      ins_ADC(MMU::read(addr));
      break;
    case 0x35:    _dbg("AND ZPX");
      zp = fetch() + _regX;
      ins_AND(MMU::read(zp));
      break;
    case 0x39:    _dbg("AND ABSY");
      addr = (fetch() | fetch() << 8) + _regY;
      ins_AND(MMU::read(addr));
      break;
    case 0x3d:    _dbg("AND ABSX");
      addr = (fetch() | fetch() << 8) + _regX;
      ins_AND(MMU::read(addr));
      break;
    /*** AND END ***/

    /*** ASL ***/
    case 0x06:    _dbg("ASL ZP");
		zp = fetch();
		val = MMU::read(zp);
		ins_ASL(&val);
		MMU::write(zp, val);
		break;
    case 0x0a:    _dbg("ASL IMPL");
		ins_ASL(&_regA);
		break;
    case 0x16:    _dbg("ASL ZPX");
		zp = fetch() + _regX;
		val = MMU::read(zp);
		ins_ASL(&val);
		MMU::write(zp, val);
		break;
    case 0x0e:    _dbg("ASL ABS");
      addr = fetch() | fetch() << 8;
      val = MMU::read(addr);
	  ins_ASL(&val);
      MMU::write(addr, val);
      break;
    case 0x1e:    _dbg("ASL ABSX");
      addr = (fetch() | fetch() << 8) + _regX;
      val = MMU::read(addr);
	  ins_ASL(&val);
      MMU::write(addr, val);
      break;
    /*** ASL END ***/
    
    /*** BIT ***/
    case 0x24:    _dbg("BIT ZP");
		zp = fetch();
		ins_BIT(MMU::read(zp));
		break;
    case 0x2c:    _dbg("BIT ABS");
		addr = fetch() | fetch() << 8;
		ins_BIT(MMU::read(addr));
		break;
    /*** BIT END ***/
    
    /*** BRANCH ***/
    case 0x10:    _dbg("BRANCH BPL");
      off = fetch();
      if (!IS_N) jump_branch(off);
      break;
    case 0x30:    _dbg("BRANCH BMI");
      off = fetch();
      if (IS_N) jump_branch(off);
      break;
    case 0x50:    _dbg("BRANCH BVC");
      off = fetch();
      if (!IS_V) jump_branch(off);
      break;
    case 0x70:    _dbg("BRANCH BVS");
      off = fetch();
      if (IS_V) jump_branch(off);
      break;
    case 0x90:    _dbg("BRANCH BCC");
      off = fetch();
      if (!IS_C) jump_branch(off);
      break;
    case 0xb0:    _dbg("BRANCH BCS");
      off = fetch();
      if (IS_C) jump_branch(off);
      break;
    case 0xd0:    _dbg("BRANCH BNE");
      off = fetch();
      if (!IS_Z) jump_branch(off);
      break;
    case 0xf0:    _dbg("BRANCH BEQ");
      off = fetch();
      if (IS_Z) jump_branch(off);
      break;
    /*** BRANCH END ***/
    
    /*** BRK ***/
    case 0x00:    _dbg("BRK");
		SET_B;
		SET_I;
		fetch(); // throw away operand
		stack_push(_regPC >> 8);
		stack_push(_regPC);
		stack_push(_regP);
		_regPC = MMU::read(0xfffe) | MMU::read(0xffff) << 8; // todo - heck this (IRQ interrupt vector)
		break;
    /*** BRK END ***/
    
    /*** CMP ***/
    case 0xc1:    _dbg("CMP INDX");
		zp = fetch() + _regX;
		addr = MMU::read(zp) + (MMU::read(zp+1) << 8);
		ins_CMP(_regA, MMU::read(addr));
		break;
    case 0xc5:    _dbg("CMP ZP");
		zp = fetch();
		ins_CMP(_regA, MMU::read(zp));
		break;
    case 0xc9:    _dbg("CMP IMM");
		ins_CMP(_regA, fetch());
		break;
    case 0xcd:    _dbg("CMP ABS");
		addr = fetch() | fetch() << 8;
		ins_CMP(_regA, MMU::read(addr));
		break;
    case 0xd1:    _dbg("CMP INDY");
		zp = fetch();
		addr = MMU::read(zp) + (MMU::read(zp + 1) << 8) + _regY;
		ins_CMP(_regA, MMU::read(addr));
		break;
    case 0xd5:    _dbg("CMP ZPX");
		zp = fetch() + _regX;
		ins_CMP(_regA, MMU::read(zp));
		break;
    case 0xd9:    _dbg("CMP ABSY");
		addr = fetch() | (fetch() << 8) + _regY;
		ins_CMP(_regA, MMU::read(addr));
      break;
    case 0xdd:    _dbg("CMP ABSX");
		addr = fetch() | (fetch() << 8) + _regX;
		ins_CMP(_regA, MMU::read(addr));
		break;
    /*** CMP END ***/
    
    /*** CPX ***/
    case 0xe0:    _dbg("CPX IMM");
		ins_CMP(_regX, fetch());
		break;
    case 0xe4:    _dbg("CPX ZP");
		zp = fetch();
		ins_CMP(_regX, MMU::read(zp));
		break;
    case 0xec:    _dbg("CPX ABS");
		addr = fetch() | fetch() << 8;
		ins_CMP(_regX, MMU::read(addr));
		break;
    /*** CPX END ***/
    
    /*** CPY ***/
    case 0xc0:    _dbg("CPY IMM");
		ins_CMP(_regY, fetch());
		break;
    case 0xc4:    _dbg("CPY ZP");
		zp = fetch();
		ins_CMP(_regY, MMU::read(zp));
		break;
    case 0xcc:    _dbg("CPY ABS");
		addr = fetch() | fetch() << 8;
		ins_CMP(_regY, MMU::read(addr));
		break;
    /*** CPY END ***/
    
    /*** DEC ***/
    case 0xc6:    _dbg("DEC ZP");
      ins_DEC(zp);
      break;
    case 0xce:    _dbg("DEC ABS");
      addr = fetch() | fetch() << 8;
      ins_DEC(addr);
      break;
    case 0xd6:    _dbg("DEC ZPX");
      zp = fetch() + _regX;
      ins_DEC(zp);
      break;
    case 0xde:    _dbg("DEC ABSX");
      addr = fetch() | fetch() << 8 + _regX;
      ins_DEC(addr);
      break;
    /*** DEC END ***/
    
    /*** EOR ***/
    case 0x41:    _dbg("EOR INDX");
		zp = fetch() + _regX;
		addr = MMU::read(zp) | MMU::read(zp+1) << 8;
		ins_EOR(MMU::read(addr));
		break;
    case 0x45:    _dbg("EOR ZP");
		zp = fetch();
		ins_EOR(MMU::read(zp));
		break;
    case 0x49:    _dbg("EOR IMM");
		ins_EOR(fetch());
		break;
    case 0x4d:    _dbg("EOR ABS");
		addr = fetch() | fetch() << 8;
		ins_EOR(MMU::read(addr));
		break;
    case 0x51:    _dbg("EOR INDY");
      zp = fetch();
      addr = (MMU::read(zp) | MMU::read(zp+1) << 8) + _regY;
      ins_EOR(MMU::read(addr));
      break;
    case 0x55:    _dbg("EOR ZPX");
      zp = fetch() + _regX;
      ins_EOR(MMU::read(zp));
      break;
    case 0x59:    _dbg("EOR ABSY");
		addr = (fetch() | fetch() << 8) + _regY;
		ins_EOR(MMU::read(addr));
		break;
    case 0x5d:    _dbg("EOR ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		ins_EOR(MMU::read(addr));
		break;
    /*** EOR END ***/
    
    /*** FLAG ***/
    case 0x18:    _dbg("FLAG CLC");
	  CLR_C;
      break;
    case 0x38:    _dbg("FLAG SEC");
	  SET_C;
      break;
    case 0x58:    _dbg("FLAG CLI");
	  CLR_I;
      break;
    case 0x78:    _dbg("FLAG SEI");
	  SET_I;
      break;
    case 0xb8:    _dbg("FLAG CLV");
	  CLR_V;
      break;
    case 0xd8:    _dbg("FLAG CLD");
	  CLR_D;
      break;
    case 0xf8:    _dbg("FLAG SED");
	  SET_D;
      break;
    /*** FLAG END ***/
    
    /*** INC ***/
    case 0xe6:    _dbg("DEC ZP");
		zp = fetch();
		ins_INC(zp);
		break;
    case 0xee:    _dbg("INC ABS");
		addr = fetch() | fetch() << 8;
		ins_INC(addr);
		break;
    case 0xf6:    _dbg("INC ZPX");
		zp = fetch() + _regX;
		ins_INC(zp);
		break;
    case 0xfe:    _dbg("INC ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		ins_INC(addr);
		break;
    /*** INC END ***/
    
    /*** JMP ***/
    case 0x4c:    _dbg("JMP ABS");
		addr = fetch() | fetch() << 8;
		_regPC = addr;
		break;
    case 0x6c:    _dbg("JMP IND");
		addr = fetch() | fetch() << 8;
		_regPC = MMU::read(addr) | MMU::read(addr + 1) << 8;
	    break;
    /*** JMP END ***/
    
    /*** JSR ***/
    case 0x20:    _dbg("JSR ABS");
		addr = _regPC - 1;
		stack_push(addr >> 8);
		stack_push(addr);
		_regPC = fetch() | fetch() << 8;
		break;
    /*** JSR END ***/
    
    /*** LDA ***/
    case 0xa1:    _dbg("LDA INDX");
		zp = fetch() + _regX;
		addr = MMU::read(zp) + MMU::read(zp + 1) << 8;
		ins_LD_(&_regA, addr);
		break;
    case 0xa5:    _dbg("LDA ZP");
		zp = fetch();
		ins_LD_(&_regA, zp);
		break;
    case 0xa9:    _dbg("LDA IMM");
		_regA = fetch();
		if (_regA) CLR_Z; else SET_Z;
		if (_regA & 0x80) SET_N; else CLR_N;
		break;
    case 0xad:    _dbg("LDA ABS");
		addr = fetch() | fetch() << 8;
		ins_LD_(&_regA, addr);
		break;
    case 0xb1:    _dbg("LDA INDY");
		zp = fetch();
		addr = (MMU::read(zp) | MMU::read(zp+1) << 8) + _regY;
		ins_LD_(&_regA, addr);
		break;
    case 0xb5:    _dbg("LDA ZPX");
		zp = fetch() + _regX;
		ins_LD_(&_regA, zp);
		break;
    case 0xb9:    _dbg("LDA ABSY");
		addr = (fetch() | fetch() << 8) + _regY;
		ins_LD_(&_regA, addr);
		break;
    case 0xbd:    _dbg("LDA ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		ins_LD_(&_regA, addr);
		break;
    /*** LDA END ***/
    
    /*** LDX ***/
    case 0xa2:    _dbg("LDX IMM");
		_regX = fetch();
		if (_regX) CLR_Z; else SET_Z;
		if (_regX & 0x80) SET_N; else CLR_N;
		break;
    case 0xa6:    _dbg("LDX ZP");
		zp = fetch();
		ins_LD_(&_regX, zp);
		break;
    case 0xb6:    _dbg("LDX ZPY");
		zp = fetch() | _regY;
		ins_LD_(&_regX, zp);
		break;
    case 0xae:    _dbg("LDX ABS");
		addr = fetch() | fetch() << 8;
		ins_LD_(&_regX, addr);
		break;
    case 0xbe:    _dbg("LDX ABSY");
		addr = (fetch() | fetch() << 8) + _regY;
		ins_LD_(&_regX, addr);
		break;
    /*** LDX END **/
    
    /*** LDY ***/
    case 0xa0:    _dbg("LDY IMM");
		_regY = fetch();
		if (_regY) CLR_Z; else SET_Z;
		if (_regY & 0x80) SET_N; else CLR_N;
		break;
    case 0xa4:    _dbg("LDY ZP");
		zp = fetch();
		ins_LD_(&_regY, zp);
		break;
    case 0xb4:    _dbg("LDY ZPX");
		zp = fetch() + _regX;
		ins_LD_(&_regY, zp);
		break;
    case 0xac:    _dbg("LDY ABS");
		addr = fetch() | fetch() << 8;
		ins_LD_(&_regY, addr);
		break;
    case 0xbc:    _dbg("LDY ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		ins_LD_(&_regY, addr);
		break;
    /*** LDY END **/

    /*** LSR ***/
    case 0x46:    _dbg("LSR ZP");
		zp = fetch();
		val = MMU::read(zp);
		ins_LSR(&val);
		MMU::write(zp, val);
		break;
    case 0x4a:    _dbg("LSR IMPL");
		ins_LSR(&_regA);
		break;
    case 0x56:    _dbg("LSR ZPX");
		zp = fetch() + _regX;
		val = MMU::read(zp);
		ins_LSR(&val);
		MMU::write(zp, val);
		break;
    case 0x4e:    _dbg("LSR ABS");
		addr = fetch() | fetch() << 8;
		val = MMU::read(addr);
		ins_LSR(&val);
		MMU::write(addr, val);
		break;
    case 0x5e:    _dbg("LSR ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		val = MMU::read(addr);
		ins_LSR(&val);
		MMU::write(addr, val);
		break;
    /*** LSR END ***/
    
    /*** NOP ***/
    case 0xea:    _dbg("NOP");
      break;
    /*** NOP END ***/
    
    /*** ORA ***/
    case 0x01:    _dbg("ORA INDX");
		zp = fetch() + _regX;
		addr = MMU::read(zp) | MMU::read(zp + 1) << 8;
		ins_ORA(MMU::read(addr));
		break;
    case 0x05:    _dbg("ORA ZP");
		zp = fetch();
		ins_ORA(MMU::read(zp));
		break;
    case 0x09:    _dbg("ORA IMM");
		ins_ORA(fetch());
		break;
    case 0x0d:    _dbg("ORA ABS");
		addr = fetch() | fetch() << 8;
		ins_ORA(MMU::read(addr));
		break;
    case 0x11:    _dbg("ORA INDY");
		zp = fetch();
		addr = (MMU::read(zp) | MMU::read(zp + 1) << 8) + _regY;
		ins_ORA(MMU::read(addr));
		break;
    case 0x15:    _dbg("ORA ZPX");
		zp = fetch() + _regX;
		ins_ORA(MMU::read(zp));
		break;
    case 0x19:    _dbg("ORA ABSY");
		addr = (fetch() | fetch() << 8) + _regY;
		ins_ORA(MMU::read(addr));
		break;
    case 0x1d:    _dbg("ORA ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		ins_ORA(MMU::read(addr));
		break;
    /*** ORA END ***/
    
    /*** _regISTER ***/
    case 0xaa:    _dbg("TAX");
		_regX = _regA;
		if (_regX) CLR_Z; else SET_Z;
		if (_regX & 0x80) SET_N; else CLR_N;
		break;
    case 0x8a:    _dbg("TXA");
		_regA = _regX;
		if (_regA) CLR_Z; else SET_Z;
		if (_regA & 0x80) SET_N; else CLR_N;
		break;
    case 0xca:    _dbg("DEX");
		_regX--;
		if (_regX) CLR_Z; else SET_Z;
		if (_regX & 0x80) SET_N; else CLR_N;
		break;
    case 0xe8:    _dbg("INX");
		_regX += 1;
		if (_regX) CLR_Z; else SET_Z;
		if (_regX & 0x80) SET_N; else CLR_N;
		break;
    case 0xa8:    _dbg("TAY");
		_regY = _regA;
		if (_regY) CLR_Z; else SET_Z;
		if (_regY & 0x80) SET_N; else CLR_N;
		break;
    case 0x98:    _dbg("TYA");
		_regA = _regY;
		if (_regA) CLR_Z; else SET_Z;
		if (_regA & 0x80) SET_N; else CLR_N;
		break;
    case 0x88:    _dbg("DEY");
		_regY--;
		if (_regY) CLR_Z; else SET_Z;
		if (_regY & 0x80) SET_N; else CLR_N;
		break;
    case 0xc8:    _dbg("INY");
		_regY += 1;
		if (_regY) CLR_Z; else SET_Z;
		if (_regY & 0x80) SET_N; else CLR_N;
		break;
    /*** _regISTER END ***/
    
    /*** ROL ***/
    case 0x26:    _dbg("ROL ZP");
		zp = fetch();
		val = MMU::read(zp);
		ins_ROL(&val);
		MMU::write(zp, val);
		break;
    case 0x2a:    _dbg("ROL IMPL");
		ins_ROL(&_regA);
		break;
    case 0x2e:    _dbg("ROL ABS");
		addr = fetch() | fetch() << 8;
		val = MMU::read(addr);
		ins_ROL(&val);
		MMU::write(addr, val);
		break;
    case 0x36:    _dbg("ROL ZPX");
		zp = fetch() + _regX;
		val = MMU::read(zp);
		ins_ROL(&val);
		MMU::write(zp, val);
		break;
    case 0x3e:    _dbg("ROL ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		val = MMU::read(addr);
		ins_ROL(&val);
		MMU::write(addr, val);
		break;
    /*** ROL END ***/
    
    /*** ROR ***/
    case 0x66:    _dbg("ROR ZP");
		zp = fetch();
		val = MMU::read(zp);
		ins_ROR(&val);
		MMU::write(zp, val);
		break;
    case 0x6a:    _dbg("ROR IMPL");
		ins_ROR(&_regA);
		break;
    case 0x6e:    _dbg("ROR ABS");
		addr = fetch() | fetch() << 8;
		val = MMU::read(addr);
		ins_ROR(&val);
		MMU::write(addr, val);
		break;
    case 0x76:    _dbg("ROR ZPX");
		zp = fetch() + _regX;
		val = MMU::read(zp);
		ins_ROR(&val);
		MMU::write(zp, val);
		break;
    case 0x7e:    _dbg("ROR ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		val = MMU::read(addr);
		ins_ROR(&val);
		MMU::write(addr, val);
		break;
    /*** ROR END ***/
    
    /*** RTI ***/
    case 0x40:    _dbg("RTI");
		_regPC = stack_pop() << 8 | stack_pop();
		_regP = stack_pop();
      break;
    /*** RTI END ***/
    
    /*** RTS ***/
    case 0x60:    _dbg("RTS");
		_regPC = (stack_pop() | stack_pop() << 8) + 1;
		break;
    /*** RTS END ***/
    
    /*** SBC ***/
    case 0xe1:    _dbg("SBC INDX");
		zp = fetch() + _regX;
		addr = MMU::read(zp) + MMU::read(zp + 1) << 8;
		val = MMU::read(addr);
		ins_SBC(val);
		break;
    case 0xe5:    _dbg("SBC ZP");
		zp = fetch();
		ins_SBC(MMU::read(zp));
		break;
    case 0xe9:    _dbg("SBC IMM");
		ins_SBC(fetch());
		break;
    case 0xed:    _dbg("SBC ABS");
		addr = fetch() | fetch() << 8;
		ins_SBC(MMU::read(addr));
		break;
    case 0xf1:    _dbg("SBC INDY");
		zp = fetch();
		addr = (MMU::read(zp) | MMU::read(zp + 1) << 8) + _regY;
		val = MMU::read(addr);
		ins_SBC(val);
		break;
    case 0xf5:    _dbg("SBC ZPX");
		zp = fetch() + _regX;
		val = MMU::read(zp);
		ins_SBC(val);
		break;
    case 0xf9:    _dbg("SBC ABSY");
		addr = (fetch() | fetch() << 8) + _regY;
		val = MMU::read(addr);
		ins_SBC(val);
		break;
    case 0xfd:    _dbg("SBC ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		val = MMU::read(addr);
		ins_SBC(val);
		break;
    /*** SBC END ***/
    
    /*** STA ***/
    case 0x81:    _dbg("STA INDX");
		zp = fetch() + _regX;
		addr = MMU::read(zp) | MMU::read(zp + 1) << 8;
		MMU::write(addr, _regA);
		break;
    case 0x85:    _dbg("STA ZP");
		zp = fetch();
		MMU::write(zp, _regA);
		break;
    case 0x8d:    _dbg("STA ABS");
		addr = fetch() | fetch() << 8;
		MMU::write(addr, _regA);
		break;
    case 0x91:    _dbg("STA INDY");
		zp = fetch();
		addr = (MMU::read(zp) | MMU::read(zp + 1) << 8) + _regY;
		MMU::write(addr, _regA);
		break;
    case 0x95:    _dbg("STA ZPX");
		zp = fetch() + _regX;
		MMU::write(zp, _regA);
		break;
    case 0x99:    _dbg("STA ABSY");
		addr = (fetch() | fetch() << 8) + _regY;
		MMU::write(addr, _regA);
		break;
    case 0x9d:    _dbg("STA ABSX");
		addr = (fetch() | fetch() << 8) + _regX;
		MMU::write(addr, _regA);
		break;
    /*** STA END ***/

    /*** STACK ***/
    case 0x08:    _dbg("STACK PHP");
		stack_push(_regP);
		break;
    case 0x28:    _dbg("STACK PLP");
		_regP = stack_pop() | 0x20;
		break;
    case 0x48:    _dbg("STACK PHA");
		stack_push(_regA);
		break;
    case 0x68:    _dbg("STACK PLA");
		_regA = stack_pop();
		if (_regA) CLR_Z; else SET_Z;
		if (_regA & 0x80) SET_N; else CLR_N;
		break;
    case 0x9a:    _dbg("TXS");
		_regS = _regX;
		break;
    case 0xba:    _dbg("TSX");
		_regX = _regS;
		if (_regX) CLR_Z; else SET_Z;
		if (_regX & 0x80) SET_N; else CLR_N;
		break;
    /*** STACK END ***/
    
    /*** STX ***/
    case 0x86:    _dbg("STX ZP");
		zp = fetch();
		MMU::write(zp, _regX);
		break;
    case 0x8e:    _dbg("STX ABS");
		addr = fetch() | fetch() << 8;
		MMU::write(addr, _regX);
		break;
    case 0x96:    _dbg("STX ZPY");
		zp = fetch() + _regY;
		MMU::write(zp, _regX);
		break;
    /*** STX END ***/
    
    /*** STY ***/
    case 0x84:    _dbg("STX ZP");
		zp = fetch();
		MMU::write(zp, _regY);
		break;
    case 0x8c:    _dbg("STX ABS");
		addr = fetch() | fetch() << 8;
		MMU::write(addr, _regY);
		break;
    case 0x94:    _dbg("STX ZPX");
		zp = fetch() + _regX;
		MMU::write(zp, _regY);
		break;
    /*** STY END ***/
      
    default:
		_dbg("Unknown ins!");
		Serial.println("CPU Error: Illegal instruction!");
		Serial.print("pc = $");Serial.println(_regPC, HEX);
		Serial.print("sp = $");Serial.println(_regPC, HEX);
		Serial.print("p = $");Serial.print(_regPC, HEX);Serial.print(' ');
		Serial.print(IS_N ? 'N' : '-');
		Serial.print(IS_V ? 'V' : '-');
		Serial.print('*');
		Serial.print(IS_B ? 'B' : '-');
		Serial.print(IS_D ? 'D' : '-');
		Serial.print(IS_I ? 'I' : '-');
		Serial.print(IS_Z ? 'Z' : '-');
		Serial.println(IS_C ? 'C' : '-');
		Serial.println("a = $");Serial.println(_regA, HEX);
		Serial.println("x = $");Serial.println(_regX, HEX);
		Serial.println("y = $");Serial.println(_regY, HEX);
		Serial.println("<execution halted>");
		while (1);
  }

  _cycleCntr++;
}

CPU_6502::CPU_6502() {
}