#include "VIC.h"


void VIC::init()
{

}

byte VIC::peek(byte addr)
{
	byte val = 0;

	switch (addr % 0x40) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		val = addr % 2 ? _spPosY[addr / 2] : _spPosX[addr / 2];
		break;
	case 0x10:
		for (int i = 0; i < 8; i++)
			val |= (_spPosX[i] >> 7) << i;
		break;
	case 0x11:
		val = (_control[0] & 0x7f) | (_rasterPos & 0x100) >> 1;
		break;
	case 0x12:
		val = _rasterPos;
		break;
	case 0x13:
		val = _latchPosX;
		break;
	case 0x14:
		val = _latchPosY;
		break;
	case 0x15:
		val = _spDisplayEn;
		break;
	case 0x16:
		val = _control[1] | 0xc0;
		break;
	case 0x17:
		val = _spExpandY;
		break;
	case 0x18:
		val = _mcr | 0x01;
		break;
	case 0x19:
		val = _irr | 0x70;
		break;
	case 0x1a:
		val = _imr | 0xf0;
		break;
	case 0x1b:
		val = _sp2bgDisplayPriority;
		break;
	case 0x1c:
		val = _spModeSelect;
		break;
	case 0x1d:
		val = _spExpandX;
		break;
	case 0x1e:
		val = _sp2spCollisionDetect;
		_sp2spCollisionDetect = 0;
		break;
	case 0x1f:
		val = _sp2bgCollisionDetect;
		_sp2bgCollisionDetect = 0;
		break;
	case 0x20:
		val = _borderColor | 0xc0;
		break;
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
		val = _bgColor[(addr & 0xf) - 1] | 0xf0;
		break;
	case 0x25:
	case 0x26:
		val = _sp[(addr & 0xf) - 5] | 0xf0;
		break;
	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
		val = _spColor[(addr & 0xf) - 7] | 0xf0;
		break;
	default:
		val = 0xff;
	}

	return val;
}

void VIC::poke(byte addr, byte val)
{
	switch (addr % 0x40) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		if (addr % 2) _spPosY[addr / 2] = val;
		else _spPosX[addr / 2] = val;
		break;
	case 0x10:
		for (int i = 0, j = 1; i < 8; i++, j <<= 1)
			if (val & j) _spPosX[i] |= 0x100;
			else _spPosX[i] &= 0xff;
		break;
	case 0x11:
		_control[0] = val;
		
			// todo
		if (RSEL) {
		} else {
		}

		if (_rasterPos == 0x30 && DEN)
			_badLinesEn = true;

		_isBadLine = (_rasterPos >= FIRST_DMA_LINE &&
					  _rasterPos <= LAST_DMA_LINE &&
					  _rasterPos & 7 == YSCROLL &&
					  _badLinesEn);
		break;
	case 0x12:
		_rasterPos = val;
		//todo
		break;
	case 0x13:
		//_latchPosX = val;
		break;
	case 0x14:
		//_latchPosY = val;
		break;
	case 0x15:
		_spDisplayEn = val;
		break;
	case 0x16:
		_control[1] = val;
		break;
	case 0x17:
		_spExpandY = val;
		// todo
		break;
	case 0x18:
		_mcr = val;
		break;
	case 0x19:
		_irr &= ~val & 0xf;

		if (_irr & _imr) {
			_irr |= 0x80;
		} else {
			// todo cpu clear vic irq
		}
		break;
	case 0x1a:
		_imr = val & 0xf;

		if (_irr & _imr) {
			_irr |= 0x80;
			// todo cpu trigger vic irq
		} else {
			_irr &= 0x7f;
			// todo cpu clear vic irq
		}
		break;
		break;
	case 0x1b:
		_sp2bgDisplayPriority = val;
		break;
	case 0x1c:
		_spModeSelect = val;
		break;
	case 0x1d:
		_spExpandX = val;
		break;
	case 0x1e:
	case 0x1f:
		/* cannot be read */
		break;
	case 0x20:
		_borderColor = _colorMatrix[val];
		break;
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
		_bgColor[(addr & 0xf) - 1] = _colorMatrix[val];
		break;
	case 0x25:
	case 0x26:
		_sp[(addr & 0xf) - 5] = _colorMatrix[val];
		break;
	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
		_spColor[(addr & 0xf) - 7] = _colorMatrix[val];
		break;
	}
}

void VIC::changeVA(int va)
{
	_ciaVbase = va << 14;
	poke(0x18, _vbase);
}