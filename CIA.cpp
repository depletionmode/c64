#include "CIA.h"

void CIA::init()
{
	for (int i = 0; i < 2; i++) {
		_pA[i] = _pB[i] = 0;
		_ddrA[i] = _ddrB[i] = 0;

		_crA[i] = _crB[i] = 0;
		_sdr[i] = 0;
		_icr[i] = 0;
		_intMask[i] = 0;

		_tA[i] = _tB[i] = 0xffff;
		_ltchA[i] = _ltchB[i] = 1;

		_todHalt[i] = false;
		_todDiv[i] = 0;
		
		_todF[i] = _todS[i] = _todM[i] = _todH[i] = 0;
		_almF[i] = _almS[i] = _almM[i] = _almH[i] = 0;
	}

	for (int i = 0; i < 8; i++)
		_revMatrix[i] = _keyMatrix[i] = 0xff;

	_jstick = 0xff;
}

void CIA::write(enum chip chipNo, unsigned int addr, byte val)
{
	switch(addr) {
	case DPA:
		{
			_pA[chipNo] = val;
		if (chipNo == CIA2) {
			// todo: set VIC memory bank addr (~(_pA | ~_ddrA) & 0x03)
		}
		}
		break;
	case DPB:
		_pB[chipNo] = val;
		if (chipNo == CIA1) {
			//check_lp();
		}
		break;
	case DDA:
		_ddrA[chipNo] = val;
		if (chipNo == CIA2) {
			// todo: set VIC memory bank addr (~(_pA | ~_ddrA) & 0x03)
		}
		break;
	case DDB:
		_ddrB[chipNo] = val;
		if (chipNo == CIA1) {
			//check_lp()
		}
		break;
	case TMRAL:
		_ltchA[chipNo] = _ltchA[chipNo] & 0xff00 | val;
		break;
	case TMRAH:
		_ltchA[chipNo] = val << 8 | _ltchA[chipNo] & 0xff;
	
        if (!(_crA[chipNo] & 0x01)) {
                _tA[chipNo] = _ltchA[chipNo];
        }
		break;
	case TMRBL:
		_ltchB[chipNo] = _ltchB[chipNo] & 0xff00 | val;
		break;
	case TMRBH:
		_ltchB[chipNo] = val << 8 | _ltchB[chipNo] & 0xff;
	
        if (!(_crB[chipNo] & 0x01)) {
                _tB[chipNo] = _ltchB[chipNo];
        }
		break;
	case TODF:
		if (_crB[chipNo] & 0x80)
			_almF[chipNo] = val & 0xf;
		else
			_todF[chipNo] = val & 0xf;
		break;
	case TODS:
		if (_crB[chipNo] & 0x80)
			_almS[chipNo] = val & 0xf;
		else
			_todS[chipNo] = val & 0xf;
		break;
	case TODM:
		if (_crB[chipNo] & 0x80)
			_almM[chipNo] = val & 0xf;
		else
			_todM[chipNo] = val & 0xf;
		break;
	case TODH:
		if (_crB[chipNo] & 0x80)
			_almH[chipNo] = val & 0xf;
		else
			_todH[chipNo] = val & 0xf;
		break;
	case SERIOBUF:
		_sdr[chipNo] = val;
		
        _icr[chipNo] |= 8;
        if ((_intMask[chipNo] & 8))
        {
			_icr[chipNo] |= 0x80;
            // todo cpu trigger cia irq
        }
		break;
	case INTCTL:
		if (val & 0x80)
			_intMask[chipNo] |= val & 0x7f;
		else
			_intMask[chipNo] &= ~val;

		if (_icr[chipNo] & _intMask[chipNo] & 0x1f) {
			_icr[chipNo] |= 0x80;
			if (chipNo == CIA1) {
				// todo: cpu trigger cia irq
			} else if (chipNo == CIA2) {
				// todo: cpu trigger nmi if waiting
			}
		}
		break;
	case CTLREGA:
		_hasNewCrA[chipNo] = true;
		_newCrA[chipNo] = val;
		_tA_cnt_phi2[chipNo] = val & 0x20 == 0;
		break;
	case CTLREGB:
		_hasNewCrB[chipNo] = true;
		_newCrB[chipNo] = val;
		_tB_cnt_phi2[chipNo] = val & 0x60 == 0;
		_tB_cnt_tA[chipNo] = val & 0x60 == 0x40;
		break;
	}
}

byte CIA::read(enum chip chipNo, unsigned int addr)
{
	byte val = 0;

	switch(addr) {
	case DPA:
		val = _pA[chipNo] | ~_ddrA[chipNo];
		if (chipNo == CIA1) {
			byte tst = (_pB[chipNo] | ~_ddrB[chipNo]) & getJoystickBits();
			byte bit = 1;
			for (int i = 0, bit = 1; i < 8; i++, bit *=2)
				if (!(tst & bit)) val &= _revMatrix[i];
		} else if (chipNo == CIA2) {
			val &= 0x3f;
		}
		break;
	case DPB:
		if (chipNo == CIA1) {
			val = _ddrB[chipNo];
			byte tst = (_pA[chipNo] | ~_ddrA[chipNo]);
			byte bit = 1;
			for (int i = 0, bit = 1; i < 8; i++, bit *=2)
				if (!(tst & bit)) val &= _keyMatrix[i];
			val |= _pB[chipNo] & _ddrB[chipNo];
		} else if (chipNo == CIA2) {
			val = _pB[chipNo] | ~_ddrB[chipNo];
		}
		break;
	case DDA:
		val = _ddrA[chipNo];
		break;
	case DDB:
		val = _ddrB[chipNo];
		break;
	case TMRAL:
		val = _tA[chipNo];
		break;
	case TMRAH:
		val = _tA[chipNo] >> 8;
		break;
	case TMRBL:
		val = _tB[chipNo];
		break;
	case TMRBH:
		val = _tB[chipNo] >> 8;
		break;
	case TODF:
		_todHalt[chipNo] = false;
		val = _todF[chipNo];
		break;
	case TODS:
		val = _todS[chipNo];
		break;
	case TODM:
		val = _todM[chipNo];
		break;
	case TODH:
		_todHalt[chipNo] = true;
		val = _todH[chipNo];
		break;
	case SERIOBUF:
		val = _sdr[chipNo];
	case INTCTL:
		val = _icr[chipNo];
		_icr[chipNo] = 0;
		if (chipNo = CIA1) {
			// todo cpu clear cia irq
		} else if (chipNo = CIA2) {
			// todo cpu clear nmi
		}
		break;
	case CTLREGA:
		val = _crA[chipNo];
		break;
	case CTLREGB:
		val = _crA[chipNo];
		break;
	}

	return val;
}

void CIA::doTod(enum chip chipNo)
{
	if (_todDiv[chipNo]) _todDiv[chipNo]--;
	else {
		_todDiv[chipNo] = _crA[chipNo] & 0x80 ? 4 : 5;
	}

	if (++_todF[chipNo] > 9) {
		_todF[chipNo] = 0;

		byte lo = (_todS[chipNo] & 0xf) + 1;
		byte hi = _todS[chipNo] >> 4;
		if (lo > 9) {
			lo = 0;
			hi++;
		}

		if (hi > 5) {
			_todS[chipNo] = 0;
			
			byte lo = (_todM[chipNo] & 0xf) + 1;
			byte hi = _todM[chipNo] >> 4;
			if (lo > 9) {
				lo = 0;
				hi++;
			}

			if (hi > 5) {
				_todM[chipNo] = 0;
			
				byte lo = (_todH[chipNo] & 0xf) + 1;
				byte hi = _todH[chipNo] & 1;
				_todH[chipNo] &= 0x80;

				if (lo > 9) {
					lo = 0;
					hi++;
				}

				_todH[chipNo] |= hi << 4 | lo;

				if (_todH[chipNo] & 0x1f > 0x11) {
					_todH[chipNo] &= 0x80;
					_todH[chipNo] ^= 0x80;
				}
			} else {
				_todM[chipNo] = hi << 4 | lo;
			}
		} else {
			_todS[chipNo] = hi << 4 | lo;
		}
	}

	if (_todF[chipNo] == _almF[chipNo] &&
		_todS[chipNo] == _almS[chipNo] &&
		_todM[chipNo] == _almM[chipNo] &&
		_todH[chipNo] == _almH[chipNo] ) {
		// todo triger interrupt 4
	}

}

byte CIA::getJoystickBits()
{
	byte bits = 0xff;

	return bits;
}


void CIA::triggerInt(enum chip chipNo, byte bit)
{
    _icr[chipNo] |= bit;
    if (_intMask[chipNo] & bit)
    {
        _icr[chipNo] |= 0x80;
		if (chipNo == CIA1) {
			// todo trigger cpu cia irq
		} else if (chipNo == CIA2) {
			// todo trigger cpu nmi
		}
    }
}

void CIA::exec(enum chip chipNo)
{
	bool tAUnderflow[2] = {false,false},
		 tAInt[2] = {false,false},
		 tBInt[2] = {false,false},
		 tAIRQNxt[2] = {false,false},
		 tBIRQNxt[2] = {false,false};

	static enum tmrState tAState[2] = {T_STOP,T_STOP},
						 tBState[2] = {T_STOP,T_STOP};

	switch (tAState[chipNo]) {
	case T_WAIT_THEN_COUNT:
		tAState[chipNo] = T_COUNT;
	case T_STOP:
		goto tA_idle;
	case T_LOAD_THEN_STOP:
		tAState[chipNo] = T_STOP;
		_tA[chipNo] = _ltchA[chipNo];
		goto tA_idle;
	case T_LOAD_THEN_COUNT:
		tAState[chipNo] = T_COUNT;
		_tA[chipNo] = _ltchA[chipNo];
		goto tA_idle;
	case T_LOAD_THEN_WAIT_THEN_COUNT:
		tAState[chipNo] = T_WAIT_THEN_COUNT;
		if (_tA[chipNo] == 1)
			tAInt[chipNo] = true;
		else {
			_tA[chipNo] = _ltchA[chipNo];
			goto tA_idle;
		}
		break;
	case T_COUNT_THEN_STOP:
		tAState[chipNo] = T_STOP;
	case T_COUNT:
		goto tA_cnt;
	}

tA_cnt:
	if (_tA_cnt_phi2[chipNo] || tAInt[chipNo]) {
		if (!_tA[chipNo] || !(--_tA[chipNo]) || tAInt[chipNo]) {
			if (tAState[chipNo] != T_STOP || tAInt[chipNo]) {
				_tA[chipNo] = _ltchA[chipNo];
				tAIRQNxt[chipNo] = true;
				_icr[chipNo] |= 1;

				if (_crA[chipNo] & 8) {
					_crA[chipNo] &= 0xfe;
					_newCrA[chipNo] &= 0xfe;
					tAState[chipNo] = T_LOAD_THEN_STOP;
				} else
					tAState[chipNo] = T_LOAD_THEN_COUNT;
			}
			tAUnderflow[chipNo] = true;
		}
	}

tA_idle:
	if (_hasNewCrA[chipNo]) {
		switch (tAState[chipNo]) {
		case T_STOP:
		case T_LOAD_THEN_STOP:
			if (_newCrA[chipNo] & 1)
				tAState[chipNo] = _newCrA[chipNo] & 0x10 ? T_LOAD_THEN_WAIT_THEN_COUNT : T_WAIT_THEN_COUNT;
			else
				if (_newCrA[chipNo] & 0x10)
					tAState[chipNo] = T_LOAD_THEN_STOP;
			break;
		case T_COUNT:
			if (_newCrA[chipNo] & 1) {
				if (_newCrA[chipNo] & 0x10)
					tAState[chipNo] = T_LOAD_THEN_WAIT_THEN_COUNT;
			} else {
				tAState[chipNo] = _newCrA[chipNo] & 0x10 ? T_LOAD_THEN_STOP : T_COUNT_THEN_STOP;
			}
			break;
		case T_LOAD_THEN_COUNT:
		case T_WAIT_THEN_COUNT:
			if (_newCrA[chipNo] & 1) {
				if (_newCrA[chipNo] & 8) {
					_newCrA[chipNo] &= 0xfe;
					tAState[chipNo] = T_STOP;
				} else if (_newCrA[chipNo] & 0x10) {
					tAState[chipNo] = T_LOAD_THEN_WAIT_THEN_COUNT;
				}
			} else {
				tAState[chipNo] = T_STOP;
			}
			break;
		}

		_crA[chipNo] = _newCrA[chipNo] & 0xfe;
		_hasNewCrA[chipNo] = false;
	}

	switch (tBState[chipNo]) {
	case T_WAIT_THEN_COUNT:
		tBState[chipNo] = T_COUNT;
	case T_STOP:
		goto tB_idle;
	case T_LOAD_THEN_STOP:
		tBState[chipNo] = T_STOP;
		_tB[chipNo] = _ltchB[chipNo];
		goto tB_idle;
	case T_LOAD_THEN_COUNT:
		tBState[chipNo] = T_COUNT;
		_tB[chipNo] = _ltchB[chipNo];
		goto tB_idle;
	case T_LOAD_THEN_WAIT_THEN_COUNT:
		tBState[chipNo] = T_WAIT_THEN_COUNT;
		if (_tB[chipNo] == 1)
			tBInt[chipNo] = true;
		else {
			_tB[chipNo] = _ltchB[chipNo];
			goto tB_idle;
		}
		break;
	case T_COUNT_THEN_STOP:
		tBState[chipNo] = T_STOP;
	case T_COUNT:
		goto tB_cnt;
	}

tB_cnt:
	if (_tB_cnt_phi2[chipNo] || (_tB_cnt_tA[chipNo] && tAUnderflow[chipNo]) || tBInt[chipNo]) {
		if (!_tB[chipNo] || !(--_tB[chipNo])) {
			if (tBState[chipNo] != T_STOP || tBInt[chipNo]) {
				_tB[chipNo] = _ltchB[chipNo];
				tBIRQNxt[chipNo] = true;
				_icr[chipNo] |= 2;

				if (_crB[chipNo] & 8) {
					_crB[chipNo] &= 0xfe;
					_newCrB[chipNo] &= 0xfe;
					tBState[chipNo] = T_LOAD_THEN_STOP;
				} else
					tBState[chipNo] = T_LOAD_THEN_COUNT;
			}
		}
	}

tB_idle:
	if (_hasNewCrB[chipNo]) {
		switch (tBState[chipNo]) {
		case T_STOP:
		case T_LOAD_THEN_STOP:
			if (_newCrB[chipNo] & 1)
				tBState[chipNo] = _newCrB[chipNo] & 0x10 ? T_LOAD_THEN_WAIT_THEN_COUNT : T_WAIT_THEN_COUNT;
			else
				if (_newCrB[chipNo] & 0x10)
					tBState[chipNo] = T_LOAD_THEN_STOP;
			break;
		case T_COUNT:
			if (_newCrB[chipNo] & 1) {
				if (_newCrB[chipNo] & 0x10)
					tBState[chipNo] = T_LOAD_THEN_WAIT_THEN_COUNT;
			} else {
				tBState[chipNo] = _newCrB[chipNo] & 0x10 ? T_LOAD_THEN_STOP : T_COUNT_THEN_STOP;
			}
			break;
		case T_LOAD_THEN_COUNT:
		case T_WAIT_THEN_COUNT:
			if (_newCrB[chipNo] & 1) {
				if (_newCrB[chipNo] & 8) {
					_newCrB[chipNo] &= 0xfe;
					tBState[chipNo] = T_STOP;
				} else if (_newCrB[chipNo] & 0x10) {
					tBState[chipNo] = T_LOAD_THEN_WAIT_THEN_COUNT;
				}
			} else {
				tBState[chipNo] = T_STOP;
			}
			break;
		}

		_crB[chipNo] = _newCrB[chipNo] & 0xfe;
		_hasNewCrB[chipNo] = false;
	}
}

/* o compiler; pls allocate mem */
byte CIA::_pA[2] = {0,0};
byte CIA::_pB[2] = {0,0};

byte CIA::_crA[2] = {0,0};
byte CIA::_crB[2] = {0,0};
byte CIA::_ddrA[2] = {0,0};
byte CIA::_ddrB[2] = {0,0};

unsigned int CIA::_ltchA[2] = {0,0};
unsigned int CIA::_ltchB[2] = {0,0};

byte CIA::_sdr[2] = {0,0};
byte CIA::_icr[2] = {0,0};
byte CIA::_intMask[2] = {0,0};

unsigned int CIA::_tA[2] = {0,0};
unsigned int CIA::_tB[2] = {0,0};

bool CIA::_todHalt[2] = {false,false};
int CIA::_todDiv[2] = {0,0};
		
byte CIA::_todF[2] = {0,0};
byte CIA::_todS[2] = {0,0};
byte CIA::_todM[2] = {0,0};
byte CIA::_todH[2] = {0,0};
byte CIA::_almF[2] = {0,0};
byte CIA::_almS[2] = {0,0};
byte CIA::_almM[2] = {0,0};
byte CIA::_almH[2] = {0,0};

byte CIA::_jstick = 0;
byte CIA::_revMatrix[8] = {0,0,0,0,0,0,0,0};
byte CIA::_keyMatrix[8] = {0,0,0,0,0,0,0,0};

bool CIA::_hasNewCrA[2] = {false,false};
bool CIA::_hasNewCrB[2] = {false,false};
byte CIA::_newCrA[2] = {0,0};
byte CIA::_newCrB[2] = {0,0};
bool CIA::_tA_cnt_phi2[2] = {false,false};
bool CIA::_tB_cnt_phi2[2] = {false,false};
bool CIA::_tB_cnt_tA[2] = {false,false};
