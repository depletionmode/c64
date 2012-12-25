#pragma once

// Unified CIAs (CIA1 and CIA2)
#include <Arduino.h>

#define DPA 0x00
#define DPB	0x01
#define DDA 0x02
#define DDB 0x03
#define TMRAL 0x04
#define TMRAH 0x05
#define TMRBL 0x06
#define TMRBH 0x07
#define TODF 0x08
#define TODS 0x09
#define TODM 0x0a
#define TODH 0x0b
#define SERIOBUF 0x0c
#define INTCTL 0x0d
#define CTLREGA 0x0e
#define CTLREGB 0x0f

class CIA
{
public:
	enum chip {
		CIA1,
		CIA2
	};

	static void init();

	static void write(enum chip chipNo, unsigned int addr, byte val);
	static byte read(enum chip chipNo, unsigned int addr);

	static void exec(enum chip chipNo);

private:
	static void doTod(enum chip chipNo);

	static byte _pA[2], _pB[2];

	static byte _crA[2], _crB[2];
	static byte _ddrA[2], _ddrB[2];

	static unsigned int _ltchA[2], _ltchB[2];

	static byte _sdr[2];
	static byte _icr[2];
	static byte _intMask[2];

	static unsigned int _tA[2], _tB[2];

	static bool _todHalt[2];
	static int _todDiv[2];
		
	static byte _todF[2], _todS[2], _todM[2], _todH[2];
	static byte _almF[2], _almS[2], _almM[2], _almH[2];

	static byte _jstick;
	static byte _revMatrix[8];
	static byte _keyMatrix[8];

	/* other stuff... */
	static bool _hasNewCrA[2], _hasNewCrB[2];
	static byte _newCrA[2], _newCrB[2];
	static bool _tA_cnt_phi2[2], _tB_cnt_phi2[2];
	static bool _tB_cnt_tA[2];

	static byte getJoystickBits();
	static void triggerInt(enum chip chipNo, byte bit);

    enum tmrState {
        T_STOP,
        T_WAIT_THEN_COUNT,
        T_LOAD_THEN_STOP,
        T_LOAD_THEN_COUNT,
        T_LOAD_THEN_WAIT_THEN_COUNT,
        T_COUNT,
        T_COUNT_THEN_STOP,
    };
};
