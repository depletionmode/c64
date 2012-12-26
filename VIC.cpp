#include "VIC.h"
#include "MMU.h"

void VIC::init(CPU_6502 *cpu)
{
	_cpu = cpu;

	_cycle = 1;
}

void VIC::exec()
{
	switch (_cycle) {
	case 1:
		break;
	case 2:
		break;
	case 12:
		break;
	case 13:
		break;
	case 14:
		break;
	case 15:
		break;
	case 16:
		break;
	case 17:
		break;
	case 18:
		break;
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
		break;
	case 55:
		break;
	case 56:
		break;
	case 57:
		break;
	case 58:
		break;
	case 59:
		break;
	case 60:
		break;
	case 61:
		break;
	case 62:
		break;
	case 63:
		_cycle = 1;
		break;
	}

	_rasterX += 8;
	_cycle++;
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

int VIC::_rasterX;
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

long VIC::_firstBaCycle;
CPU_6502 *VIC::_cpu;

byte VIC::_cycle;