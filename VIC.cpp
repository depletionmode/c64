#include "VIC.h"
#include "MMU.h"

void VIC::init(CPU_6502 *cpu)
{
	_cpu = cpu;
}

byte VIC::peek(byte addr)
{
	byte val = 0;

	switch (addr % 0x40) {
	case 0x11:
		val = (_control[0] & 0x7f) | (_rasterY & 0x100) >> 1;
		break;
	case 0x12:
		val = _rasterY;
		break;
	case 0x16:
		val = _control[1] | 0xc0;
		break;
	default:
		val = 0xff;
	}

	return val;
}

void VIC::poke(byte addr, byte val)
{
	switch (addr % 0x40) {
	case 0x11:
		_control[0] = val;

		break;
	case 0x16:
		_control[1] = val;
		break;
	case 0x18:
		_mcr = val;
		_matrixBase = VM << 14;
		_charBase = CB << 7;
		break;
	case 0x20:
		_borderColor = _colorMatrix[val];
		break;
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
		_bgColor[(addr & 0xf) - 1] = _colorMatrix[val & 0xf];
		break;
	}
}

byte VIC::read_mmu(int addr)
{
	/* virtual address translation as VIC can access full memory space in 4 banks */
	int va = addr; // todo when cia is integrated | _ciaVaBase;
	if (addr & 0x7000 == 0x1000)
		return MMU::read(0xd0 << 8 | (va & 0xfff));
	else
		return MMU::read(va);
}

void VIC::accessMatrix()
{
	if (!_cpu->getBa()) {	// if BA is LOW
		if (_cpu->getCycle() - _firstBaCycle < 3)	// we're in the 1st 3 cycles
			_lineMatrix[_lineIdx] = _lineColor[_lineIdx] = 0xff;
		else	// we're doing our stuff
		{
			int addr = (_vc & 0x3ff) | _matrixBase;
			_lineMatrix[_lineIdx] = read_mmu(addr);
			_lineColor[_lineIdx] = MMU::read(0xd8 << 8 | addr & 0x3ff); // get from color ram
		}
	}
}

void VIC::accessGraphics()
{
	if (_displayState) {
		int addr =  _lineMatrix[_lineIdx] << 3 | _charBase | _rc;

		_dataGfx = read_mmu(addr);
		_dataChar = _lineMatrix[_lineIdx];
		_dataColor = _colorMatrix[_lineIdx];

		_lineIdx++;
		_vc++;
	}
}

/* o compiler; please allocate mem */
bool VIC::_displayState;

int VIC::_vc,
		 _vcbase;
int VIC::_rc;

/* mapped registers */
byte VIC::_spPosX[8];
byte VIC::_spPosY[8];
byte VIC::_control[2];
byte VIC::_rasterPos;
byte VIC::_latchPosX;
byte VIC::_latchPosY;
byte VIC::_spDisplayEn;
byte VIC::_spExpandX;
byte VIC::_spExpandY;
byte VIC::_mcr;
byte VIC::_irr, _imr;
byte VIC::_sp2bgDisplayPriority;
byte VIC::_spModeSelect;
byte VIC::_sp2spCollisionDetect;
byte VIC::_sp2bgCollisionDetect;
byte VIC::_borderColor;
byte VIC::_bgColor[4];
byte VIC::_sp[2];
byte VIC::_spColor;

int VIC::_rasterY;
int VIC::_matrixBase;
int VIC::_charBase;
byte VIC::_colorMatrix[16];

byte VIC::_dataGfx,
			_dataChar,
			_dataColor;

int VIC::_lineIdx;
byte VIC::_lineMatrix[40],
		  _lineColor[40];

long _firstBaCycle;
CPU_6502 *_cpu;