#pragma once

#include <Arduino.h>
#include "CPU_6502.h"

class VIC
{
public:
	static void init();
	static void exec();
	
	/* addr in range $00 - $2e */
	static byte peek(byte addr);
	static void poke(byte addr, byte val);

private:
	static byte read_mmu(int addr);

	static void accessMatrix();
	static void accessGraphics();

	/* colour palette */
	enum colors {
		BLACK,
		WHITE,
		RED,
		CYAN,
		PINK,
		GREEN,
		BLUE,
		YELLOW,
		ORANGE,
		BROWN,
		LIGHT_RED,
		DARK_GREY,
		MEDIUM_GREY,
		LIGHT_GREEN,
		LIGHT_BLUE,
		LIGHT_GREY
	};

#define DISPLAY_IDX ((_control[0] & 0x60 | _control[1] & 0x10) >> 4)

	/* control register 1 */
#define YSCROLL (_control[0] & 7)
#define RSEL ((_control[0] & 8) >> 3)
#define DEN ((_control[0] & 0x10) >> 4)
#define BMM ((_control[0] & 0x20) >> 5)
#define ECM ((_control[0] & 0x40) >> 6)
#define RST8 ((_control[0] & 0x80) >> 7)

	/* control register 2 */
#define XSCROLL (_control[1] & 7)
#define CSEL ((_control[1] & 8) >> 3)
#define MCM ((_control[1] & 0x10) >> 4)
#define RES ((_control[1] & 0x20) >> 5 )/* should always be 0! */

	/* memory control register (mcr) */ // vbase
#define SEL_CHARSET (_mcr & 1)
#define CB ((_mcr & 0xe) >> 1)  // char base << 7
#define VM ((_mcr & 0xf0) >> 4) // matrix base << 14

	/* interrupt flag register (irr) */
#define IRST (_irr & 1)
#define IMBC ((_irr & 2) >> 1)
#define IMMC ((_irr & 3) >> 2)
#define ILP ((_irr & 4) >> 3)
#define IRQ ((_irr & 0x80) >> 7)

	/* interrupt mask register (imr) */
#define ERST (_imr & 1)
#define EMBC ((_imr & 2) >> 1)
#define EMMC ((_imr & 3) >> 2)
#define ELP ((_imr & 4) >> 3)

	/* internal registers */
	static bool _displayState;

	static int _vc,			/* video counter */
			   _vcbase;		/* video counter base */
	static int _rc;			/* row counter */
	//static int _vmli;		/* video matrix line idx */

	/* mapped registers */
	static byte _spPosX[8];
	static byte _spPosY[8];
	static byte _control[2];
	static byte _rasterPos;
	static byte _latchPosX;
	static byte _latchPosY;
	static byte _spDisplayEn;
	static byte _spExpandX;
	static byte _spExpandY;
	static byte _mcr;
	static byte _irr, _imr;
	static byte _sp2bgDisplayPriority;
	static byte _spModeSelect;
	static byte _sp2spCollisionDetect;
	static byte _sp2bgCollisionDetect;
	static byte _borderColor;
	static byte _bgColor[4];
	static byte _sp[2];
	static byte _spColor;

	static int _rasterY;
	static int _matrixBase;
	static int _charBase;
	static byte _colorMatrix[16];

	static byte _dataGfx,
				_dataChar,
				_dataColor;

	static int _lineIdx;
	static byte _lineMatrix[40],
				_lineColor[40];

	static long _firstBaCycle;
	static CPU_6502 *_cpu;
};

