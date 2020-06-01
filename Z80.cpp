/*
	Copyright (c) 2020 Paul Espina

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "Z80.h"

#include <fstream>

Z80::Z80()
{
	IME = false;
	memset(&registers, 0, sizeof(registers));
	sp = 0;
	pc = 0;
	memset(&cartridge, 0, sizeof(cartridge));
	memset(&memory, 0, sizeof(memory));
	memset(&screen, 0, sizeof(screen));
	cycle_count = 0;
	Init();
}

bool Z80::LoadCartridge(std::string path)
{
	std::ifstream file(path, std::ifstream::binary | std::ifstream::in);
	if(file.is_open())
	{
		return false;
	}
	file.seekg(0, std::ios::end);
	std::streamoff length = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> result((unsigned int) length);
	file.read(&result[0], length);
	for(unsigned int i = 0, size = result.size(); i < size; i++)
	{
		cartridge[i] = (uint8_t) result[i];
	}
	return true;
}

void Z80::Init()
{
	registers[0] = 0x01b0;
	registers[1] = 0x0013;
	registers[2] = 0x00d8;
	registers[3] = 0x014d;
	sp = 0xfffe;
	pc = 0x100;
	memset(&memory, 0, sizeof(memory));
	memset(&screen, 0, sizeof(screen));
}

uint8_t Z80::ReadMem(uint16_t addr)
{
	if(addr >= 0 && addr <= 0xbfff)
	{
		return memory[addr];
	}
}

void Z80::WriteMem(uint16_t addr, uint8_t data)
{
	if(addr >= 0xa000 && addr <= 0x7fff)
	{
		memory[addr] = data;
	}
}

uint8_t Z80::GetHiRegister(uint16_t reg)
{
	return reg >> 8;
}

uint8_t Z80::GetLoRegister(uint16_t reg)
{
	return reg & 0x00ff;
}

void Z80::SetHiRegister(uint16_t &reg, uint8_t data)
{
	uint16_t hi = data;
	hi <<= 8;
	uint8_t lo = GetLoRegister(reg);
	reg = hi | lo;
}

void Z80::SetLoRegister(uint16_t &reg, uint8_t data)
{
	uint16_t hi = GetHiRegister(reg);
	hi <<= 8;
	reg = hi | data;
}

void Z80::SetFlag(int bit, bool value)
{
	uint8_t f = GetLoRegister(registers[AF]);
	uint8_t setter = 1 << (bit + 4);
	if(value)
	{
		f |= setter;
	}
	else
	{
		setter ^= 0xff;
		f &= setter;
	}
}

bool Z80::GetFlag(int bit)
{
	uint8_t f = GetLoRegister(registers[AF]);
	uint8_t setter = 1 << (bit + 4);
	return (f & setter) >> (bit + 4);
}

void Z80::Cycle()
{
	if(cycle_count > 0)
	{
		uint8_t opcode = Fetch();
		cycle_count = Decode(opcode);
	}
	cycle_count--;
}

uint8_t Z80::Fetch()
{
	uint8_t opcode = memory[pc];
	pc++;
	return opcode;
}

uint8_t Z80::Decode(uint8_t opcode)
{
	uint8_t count = 0;
	uint16_t nn;
	uint8_t n;
	switch(opcode)
	{
	// 0x
	case 0x00:
		count = 4;
		break;
	case 0x01:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		LD16(registers[BC], nn);
		count = 12;
		break;
	case 0x02:
		LD8(registers[BC], GetHiRegister(registers[AF]));
		count = 8;
		break;
	case 0x03:
		INC16(registers[BC]);
		count = 8;
		break;
	case 0x04:
		INC8(registers[BC], "hi");
		count = 4;
		break;
	case 0x05:
		DEC8(registers[BC], "hi");
		count = 4;
		break;
	case 0x06:
		n = Fetch();
		LD8(registers[BC], "hi", n);
		count = 8;
		break;
	case 0x07:
		RLCA();
		count = 4;
		break;
	case 0x08:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		LD8(nn++, GetHiRegister(sp));
		LD8(nn, GetLoRegister(sp));
		count = 20;
		break;
	case 0x09:
		ADD16(registers[BC]);
		count = 8;
		break;
	case 0x0a:
		n = ReadMem(registers[BC]);
		LD8(registers[AF], "hi", n);
		count = 8;
		break;
	case 0x0b:
		DEC16(registers[BC]);
		count = 8;
		break;
	case 0x0c:
		INC8(registers[BC], "lo");
		count = 4;
		break;
	case 0x0d:
		DEC8(registers[BC], "lo");
		count = 4;
		break;
	case 0x0e:
		n = Fetch();
		LD8(registers[BC], "lo", n);
		count = 8;
		break;
	case 0x0f:
		RRCA();
		count = 4;
		break;
	// 1x
	case 0x10:
		STOP();
		count = 4;
		break;
	case 0x11:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		LD16(registers[DE], nn);
		count = 12;
		break;
	case 0x12:
		LD8(registers[DE], GetHiRegister(registers[AF]));
		count = 8;
		break;
	case 0x13:
		INC16(registers[DE]);
		count = 8;
		break;
	case 0x14:
		INC8(registers[DE], "hi");
		count = 4;
		break;
	case 0x15:
		DEC8(registers[DE], "hi");
		count = 4;
		break;
	case 0x16:
		n = Fetch();
		LD8(registers[DE], "hi", n);
		count = 8;
		break;
	case 0x17:
		RLA();
		count = 4;
		break;
	case 0x18:
		n = Fetch();
		JR(n);
		count = 12;
		break;
	case 0x19:
		ADD16(registers[DE]);
		count = 8;
		break;
	case 0x1a:
		n = ReadMem(registers[DE]);
		LD8(registers[AF], "hi", n);
		count = 8;
		break;
	case 0x1b:
		DEC16(registers[DE]);
		count = 8;
		break;
	case 0x1c:
		INC8(registers[DE], "lo");
		count = 4;
		break;
	case 0x1d:
		DEC8(registers[DE], "lo");
		count = 4;
		break;
	case 0x1e:
		n = Fetch();
		LD8(registers[DE], "lo", n);
		count = 8;
		break;
	case 0x1f:
		RRA();
		count = 4;
		break;
	// 2x
	case 0x20:
		n = Fetch();
		count = JR(rel_jp_flag::NZ, n);
		count += 8;
		break;
	case 0x21:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		LD16(registers[HL], nn);
		count = 12;
		break;
	case 0x22:
		LD8(registers[HL], GetHiRegister(registers[AF]));
		INC16(registers[HL]);
		count = 8;
		break;
	case 0x23:
		INC16(registers[HL]);
		count = 8;
		break;
	case 0x24:
		INC8(registers[HL], "hi");
		count = 4;
		break;
	case 0x25:
		DEC8(registers[HL], "hi");
		count = 4;
		break;
	case 0x26:
		n = Fetch();
		LD8(registers[HL], "hi", n);
		count = 8;
		break;
	case 0x27:
		DAA();
		count = 4;
		break;
	case 0x28:
		n = Fetch();
		count = JR(rel_jp_flag::Z, n);
		count += 8;
		break;
	case 0x29:
		ADD16(registers[HL]);
		count = 8;
		break;
	case 0x2a:
		n = ReadMem(registers[HL]);
		LD8(registers[AF], "hi", n);
		INC16(registers[HL]);
		count = 8;
		break;
	case 0x2b:
		DEC16(registers[HL]);
		count = 8;
		break;
	case 0x2c:
		INC8(registers[HL], "lo");
		count = 4;
		break;
	case 0x2d:
		DEC8(registers[HL], "lo");
		count = 4;
		break;
	case 0x2e:
		n = Fetch();
		LD8(registers[HL], "lo", n);
		count = 8;
		break;
	case 0x2f:
		CPL();
		count = 4;
		break;
	// 3x
	case 0x30:
		n = Fetch();
		count = JR(rel_jp_flag::NC, n);
		count += 8;
		break;
	case 0x31:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		LD16(sp, nn);
		count = 12;
		break;
	case 0x32:
		LD8(registers[HL], GetHiRegister(registers[AF]));
		DEC16(registers[HL]);
		count = 8;
		break;
	case 0x33:
		INC16(sp);
		count = 8;
		break;
	case 0x34:
		INC8(registers[HL]);
		count = 12;
		break;
	case 0x35:
		DEC8(registers[HL]);
		count = 12;
		break;
	case 0x36:
		n = Fetch();
		LD8(registers[HL], n);
		count = 12;
		break;
	case 0x37:
		SCF();
		count = 4;
		break;
	case 0x38:
		n = Fetch();
		count = JR(rel_jp_flag::C, n);
		count += 8;
		break;
	case 0x39:
		ADD16(sp);
		count = 8;
		break;
	case 0x3a:
		n = ReadMem(registers[HL]);
		LD8(registers[AF], "hi", n);
		DEC16(registers[HL]);
		count = 8;
		break;
	case 0x3b:
		DEC16(sp);
		count = 8;
		break;
	case 0x3c:
		INC8(registers[AF], "hi");
		count = 4;
		break;
	case 0x3d:
		DEC8(registers[AF], "hi");
		count = 4;
		break;
	case 0x3e:
		n = Fetch();
		LD8(registers[AF], "hi", n);
		count = 8;
		break;
	case 0x3f:
		CCF();
		count = 4;
		break;
	// 4x
	case 0x40:
		LD8(registers[BC], "hi", GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0x41:
		LD8(registers[BC], "hi", GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0x42:
		LD8(registers[BC], "hi", GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0x43:
		LD8(registers[BC], "hi", GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0x44:
		LD8(registers[BC], "hi", GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0x45:
		LD8(registers[BC], "hi", GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0x46:
		n = ReadMem(registers[HL]);
		LD8(registers[BC], "hi", n);
		count = 8;
		break;
	case 0x47:
		LD8(registers[BC], "hi", GetHiRegister(registers[AF]));
		count = 4;
		break;
	case 0x48:
		LD8(registers[BC], "lo", GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0x49:
		LD8(registers[BC], "lo", GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0x4a:
		LD8(registers[BC], "lo", GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0x4b:
		LD8(registers[BC], "lo", GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0x4c:
		LD8(registers[BC], "lo", GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0x4d:
		LD8(registers[BC], "lo", GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0x4e:
		n = ReadMem(registers[HL]);
		LD8(registers[BC], "lo", n);
		count = 8;
		break;
	case 0x4f:
		LD8(registers[BC], "lo", GetHiRegister(registers[AF]));
		count = 4;
		break;
	// 5x
	case 0x50:
		LD8(registers[DE], "hi", GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0x51:
		LD8(registers[DE], "hi", GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0x52:
		LD8(registers[DE], "hi", GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0x53:
		LD8(registers[DE], "hi", GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0x54:
		LD8(registers[DE], "hi", GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0x55:
		LD8(registers[DE], "hi", GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0x56:
		n = ReadMem(registers[HL]);
		LD8(registers[DE], "hi", n);
		count = 8;
		break;
	case 0x57:
		LD8(registers[DE], "lo", GetHiRegister(registers[AF]));
		count = 4;
		break;
	case 0x58:
		LD8(registers[DE], "lo", GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0x59:
		LD8(registers[DE], "lo", GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0x5a:
		LD8(registers[DE], "lo", GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0x5b:
		LD8(registers[DE], "lo", GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0x5c:
		LD8(registers[DE], "lo", GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0x5d:
		LD8(registers[DE], "lo", GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0x5e:
		n = ReadMem(registers[HL]);
		LD8(registers[DE], "lo", n);
		count = 8;
		break;
	case 0x5f:
		LD8(registers[DE], "lo", GetHiRegister(registers[AF]));
		count = 4;
		break;
	// 6x
	case 0x60:
		LD8(registers[HL], "hi", GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0x61:
		LD8(registers[HL], "hi", GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0x62:
		LD8(registers[HL], "hi", GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0x63:
		LD8(registers[HL], "hi", GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0x64:
		LD8(registers[HL], "hi", GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0x65:
		LD8(registers[HL], "hi", GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0x66:
		n = ReadMem(registers[HL]);
		LD8(registers[HL], "hi", n);
		count = 8;
		break;
	case 0x67:
		LD8(registers[HL], "lo", GetHiRegister(registers[AF]));
		count = 4;
		break;
	case 0x68:
		LD8(registers[HL], "lo", GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0x69:
		LD8(registers[HL], "lo", GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0x6a:
		LD8(registers[HL], "lo", GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0x6b:
		LD8(registers[HL], "lo", GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0x6c:
		LD8(registers[HL], "lo", GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0x6d:
		LD8(registers[HL], "lo", GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0x6e:
		n = ReadMem(registers[HL]);
		LD8(registers[HL], "lo", n);
		count = 8;
		break;
	case 0x6f:
		LD8(registers[HL], "lo", GetHiRegister(registers[AF]));
		count = 4;
		break;
	// 7x
	case 0x70:
		LD8(registers[HL], GetHiRegister(registers[BC]));
		count = 8;
		break;
	case 0x71:
		LD8(registers[HL], GetLoRegister(registers[BC]));
		count = 8;
		break;
	case 0x72:
		LD8(registers[HL], GetHiRegister(registers[DE]));
		count = 8;
		break;
	case 0x73:
		LD8(registers[HL], GetLoRegister(registers[DE]));
		count = 8;
		break;
	case 0x74:
		LD8(registers[HL], GetHiRegister(registers[HL]));
		count = 8;
		break;
	case 0x75:
		LD8(registers[HL], GetLoRegister(registers[HL]));
		count = 8;
		break;
	case 0x76:
		HALT();
		count = 4;
		break;
	case 0x77:
		LD8(registers[HL], GetHiRegister(registers[AF]));
		count = 8;
		break;
	case 0x78:
		LD8(registers[AF], "hi", GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0x79:
		LD8(registers[AF], "hi", GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0x7a:
		LD8(registers[AF], "hi", GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0x7b:
		LD8(registers[AF], "hi", GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0x7c:
		LD8(registers[AF], "hi", GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0x7d:
		LD8(registers[AF], "hi", GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0x7e:
		n = ReadMem(registers[HL]);
		LD8(registers[AF], "hi", n);
		count = 8;
		break;
	case 0x7f:
		LD8(registers[AF], "hi", GetHiRegister(registers[AF]));
		count = 4;
		break;
	// 8x
	case 0x80:
		ADD8(GetHiRegister(registers[BC]), false);
		count = 4;
		break;
	case 0x81:
		ADD8(GetLoRegister(registers[BC]), false);
		count = 4;
		break;
	case 0x82:
		ADD8(GetHiRegister(registers[DE]), false);
		count = 4;
		break;
	case 0x83:
		ADD8(GetLoRegister(registers[DE]), false);
		count = 4;
		break;
	case 0x84:
		ADD8(GetHiRegister(registers[HL]), false);
		count = 4;
		break;
	case 0x85:
		ADD8(GetLoRegister(registers[HL]), false);
		count = 4;
		break;
	case 0x86:
		n = ReadMem(registers[HL]);
		ADD8(n, false);
		count = 8;
		break;
	case 0x87:
		ADD8(GetHiRegister(registers[AF]), false);
		count = 4;
		break;
	case 0x88:
		ADD8(GetHiRegister(registers[BC]), true);
		count = 4;
		break;
	case 0x89:
		ADD8(GetLoRegister(registers[BC]), true);
		count = 4;
		break;
	case 0x8a:
		ADD8(GetHiRegister(registers[DE]), true);
		count = 4;
		break;
	case 0x8b:
		ADD8(GetLoRegister(registers[DE]), true);
		count = 4;
		break;
	case 0x8c:
		ADD8(GetHiRegister(registers[HL]), true);
		count = 4;
		break;
	case 0x8d:
		ADD8(GetLoRegister(registers[HL]), true);
		count = 4;
		break;
	case 0x8e:
		n = ReadMem(registers[HL]);
		ADD8(n, true);
		count = 8;
		break;
	case 0x8f:
		ADD8(GetHiRegister(registers[AF]), true);
		count = 4;
		break;
	// 9x
	case 0x90:
		SUB(GetHiRegister(registers[BC]), false);
		count = 4;
		break;
	case 0x91:
		SUB(GetLoRegister(registers[BC]), false);
		count = 4;
		break;
	case 0x92:
		SUB(GetHiRegister(registers[DE]), false);
		count = 4;
		break;
	case 0x93:
		SUB(GetLoRegister(registers[DE]), false);
		count = 4;
		break;
	case 0x94:
		SUB(GetHiRegister(registers[HL]), false);
		count = 4;
		break;
	case 0x95:
		SUB(GetLoRegister(registers[HL]), false);
		count = 4;
		break;
	case 0x96:
		n = ReadMem(registers[HL]);
		SUB(n, false);
		count = 8;
		break;
	case 0x97:
		SUB(GetHiRegister(registers[AF]), false);
		count = 4;
		break;
	case 0x98:
		SUB(GetHiRegister(registers[BC]), true);
		count = 4;
		break;
	case 0x99:
		SUB(GetLoRegister(registers[BC]), true);
		count = 4;
		break;
	case 0x9a:
		SUB(GetHiRegister(registers[DE]), true);
		count = 4;
		break;
	case 0x9b:
		SUB(GetLoRegister(registers[DE]), true);
		count = 4;
		break;
	case 0x9c:
		SUB(GetHiRegister(registers[HL]), true);
		count = 4;
		break;
	case 0x9d:
		SUB(GetLoRegister(registers[HL]), true);
		count = 4;
		break;
	case 0x9e:
		n = ReadMem(registers[HL]);
		SUB(n, true);
		count = 8;
		break;
	case 0x9f:
		SUB(GetHiRegister(registers[AF]), true);
		count = 4;
		break;
	// Ax
	case 0xa0:
		AND(GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0xa1:
		AND(GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0xa2:
		AND(GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0xa3:
		AND(GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0xa4:
		AND(GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0xa5:
		AND(GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0xa6:
		n = ReadMem(registers[HL]);
		AND(n);
		count = 8;
		break;
	case 0xa7:
		AND(GetHiRegister(registers[AF]));
		count = 4;
		break;
	case 0xa8:
		XOR(GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0xa9:
		XOR(GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0xaa:
		XOR(GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0xab:
		XOR(GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0xac:
		XOR(GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0xad:
		XOR(GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0xae:
		n = ReadMem(registers[HL]);
		XOR(n);
		count = 8;
		break;
	case 0xaf:
		XOR(GetHiRegister(registers[AF]));
		count = 4;
		break;
	// Bx
	case 0xb0:
		OR(GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0xb1:
		OR(GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0xb2:
		OR(GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0xb3:
		OR(GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0xb4:
		OR(GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0xb5:
		OR(GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0xb6:
		n = ReadMem(registers[HL]);
		OR(n);
		count = 8;
		break;
	case 0xb7:
		OR(GetHiRegister(registers[AF]));
		count = 4;
		break;
	case 0xb8:
		CP(GetHiRegister(registers[BC]));
		count = 4;
		break;
	case 0xb9:
		CP(GetLoRegister(registers[BC]));
		count = 4;
		break;
	case 0xba:
		CP(GetHiRegister(registers[DE]));
		count = 4;
		break;
	case 0xbb:
		CP(GetLoRegister(registers[DE]));
		count = 4;
		break;
	case 0xbc:
		CP(GetHiRegister(registers[HL]));
		count = 4;
		break;
	case 0xbd:
		CP(GetLoRegister(registers[HL]));
		count = 4;
		break;
	case 0xbe:
		n = ReadMem(registers[HL]);
		CP(n);
		count = 8;
		break;
	case 0xbf:
		CP(GetHiRegister(registers[AF]));
		count = 4;
		break;
	// Cx
	case 0xc0:
		count = RET(rel_jp_flag::NZ);
		count += 8;
		break;
	case 0xc1:
		POP(registers[BC]);
		count = 12;
		break;
	case 0xc2:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		count = JP(rel_jp_flag::NZ, nn);
		count += 12;
		break;
	case 0xc3:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		JP(nn);
		count = 16;
		break;
	case 0xc4:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		count = CALL(rel_jp_flag::NZ, nn);
		count += 12;
		break;
	case 0xc5:
		PUSH(registers[BC]);
		count = 16;
		break;
	case 0xc6:
		n = Fetch();
		ADD8(n, false);
		count = 8;
		break;
	case 0xc7:
		RST(0x0);
		count = 16;
		break;
	case 0xc8:
		count = RET(rel_jp_flag::Z);
		count += 8;
		break;
	case 0xc9:
		RET();
		count = 16;
		break;
	case 0xca:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		count = JP(rel_jp_flag::Z, nn);
		count += 12;
		break;
	case 0xcb:
		count = PrefixCB(n);
		break;
	case 0xcc:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		count = CALL(rel_jp_flag::Z, nn);
		count += 12;
		break;
	case 0xcd:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		CALL(nn);
		count = 24;
		break;
	case 0xce:
		n = Fetch();
		ADD8(n, true);
		count = 8;
		break;
	case 0xcf:
		RST(0x08);
		count = 16;
		break;
	// Dx
	case 0xd0:
		count = RET(rel_jp_flag::NC);
		count += 8;
		break;
	case 0xd1:
		POP(registers[DE]);
		count = 12;
		break;
	case 0xd2:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		count = JP(rel_jp_flag::NC, nn);
		count += 12;
	case 0xd4:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		count = CALL(rel_jp_flag::NC, nn);
		count += 12;
		break;
	case 0xd5:
		PUSH(registers[DE]);
		count = 16;
		break;
	case 0xd6:
		n = Fetch();
		SUB(n, false);
		count = 8;
		break;
	case 0xd7:
		RST(0x10);
		count = 16;
		break;
	case 0xd8:
		count = RET(rel_jp_flag::C);
		count += 8;
		break;
	case 0xd9:
		RETI();
		count = 16;
		break;
	case 0xda:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		count = JP(rel_jp_flag::C, nn);
		count += 12;
		break;
	case 0xdc:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		count = CALL(rel_jp_flag::C, nn);
		count += 12;
		break;
	case 0xde:
		n = Fetch();
		SUB(n, true);
		count = 8;
		break;
	case 0xdf:
		RST(0x18);
		count = 16;
		break;
	// Ex
	case 0xe0:
		n = Fetch();
		LD8(0xff00 + n, GetHiRegister(registers[AF]));
		count = 12;
		break;
	case 0xe1:
		POP(registers[HL]);
		count = 12;
		break;
	case 0xe2:
		LD8(0xff00 + GetLoRegister(registers[BC]), GetHiRegister(registers[AF]));
		count = 8;
		break;
	case 0xe5:
		PUSH(registers[HL]);
		count = 16;
		break;
	case 0xe6:
		n = Fetch();
		AND(n);
		count = 8;
		break;
	case 0xe7:
		RST(0x20);
		count = 16;
		break;
	case 0xe8:
		n = Fetch();
		ADD16((int8_t) n);
		count = 16;
		break;
	case 0xe9:
		JP();
		count = 4;
		break;
	case 0xea:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		LD16(nn, GetHiRegister(registers[AF]));
		count = 16;
		break;
	case 0xee:
		n = Fetch();
		XOR(n);
		count = 8;
		break;
	case 0xef:
		RST(0x28);
		count = 16;
		break;
	// Fx
	case 0xf0:
		n = Fetch();
		LD8(registers[AF], "hi", 0xff00 + n);
		count = 12;
		break;
	case 0xf1:
		POP(registers[AF]);
		count = 12;
		break;
	case 0xf2:
		LD8(registers[AF], "hi", ReadMem(0xff00 + GetLoRegister(registers[BC])));
		count = 8;
		break;
	case 0xf3:
		DI();
		count = 4;
		break;
	case 0xf5:
		PUSH(registers[AF]);
		count = 16;
		break;
	case 0xf6:
		n = Fetch();
		OR(n);
		count = 8;
		break;
	case 0xf7:
		RST(0x30);
		count = 16;
		break;
	case 0xf8:
		n = Fetch();
		LD16(registers[HL], sp + (int8_t) n);
		count = 12;
		break;
	case 0xf9:
		LD16(sp, registers[HL]);
		count = 8;
		break;
	case 0xfa:
		nn = Fetch();
		nn <<= 8;
		nn |= Fetch();
		LD8(registers[AF], "hi", ReadMem(nn));
		count = 16;
		break;
	case 0xfb:
		EI();
		count = 4;
		break;
	case 0xfe:
		n = Fetch();
		CP(n);
		count = 8;
		break;
	case 0xff:
		RST(0x38);
		count = 16;
		break;
	}
	return count;
}

uint8_t Z80::PrefixCB(uint8_t opcode)
{
	uint8_t count = 0;
	switch(opcode)
	{
	// 0x
	case 0x0:
		RLC(registers[BC], "hi");
		count = 8;
		break;
	case 0x1:
		RLC(registers[BC], "lo");
		count = 8;
		break;
	case 0x2:
		RLC(registers[DE], "hi");
		count = 8;
		break;
	case 0x3:
		RLC(registers[DE], "lo");
		count = 8;
		break;
	case 0x4:
		RLC(registers[HL], "hi");
		count = 8;
		break;
	case 0x5:
		RLC(registers[HL], "lo");
		count = 8;
		break;
	case 0x6:
		RLC();
		count = 16;
		break;
	case 0x7:
		RLC(registers[AF], "hi");
		count = 8;
		break;
	case 0x8:
		RRC(registers[BC], "hi");
		count = 8;
		break;
	case 0x9:
		RRC(registers[BC], "lo");
		count = 8;
		break;
	case 0xa:
		RRC(registers[DE], "hi");
		count = 8;
		break;
	case 0xb:
		RRC(registers[DE], "lo");
		count = 8;
		break;
	case 0xc:
		RRC(registers[HL], "hi");
		count = 8;
		break;
	case 0xd:
		RRC(registers[HL], "lo");
		count = 8;
		break;
	case 0xe:
		RLC();
		count = 16;
		break;
	case 0xf:
		RRC(registers[AF], "hi");
		count = 8;
		break;
	// 1x
	case 0x10:
		RR(registers[BC], "hi");
		count = 8;
		break;
	case 0x11:
		RR(registers[BC], "lo");
		count = 8;
		break;
	case 0x12:
		RR(registers[DE], "hi");
		count = 8;
		break;
	case 0x13:
		RR(registers[DE], "lo");
		count = 8;
		break;
	case 0x14:
		RR(registers[HL], "hi");
		count = 8;
		break;
	case 0x15:
		RR(registers[HL], "lo");
		count = 8;
		break;
	case 0x16:
		RR();
		count = 16;
		break;
	case 0x17:
		RR(registers[AF], "hi");
		count = 8;
		break;
	case 0x18:
		RR(registers[BC], "hi");
		count = 8;
		break;
	case 0x19:
		RR(registers[BC], "lo");
		count = 8;
		break;
	case 0x1a:
		RR(registers[DE], "hi");
		count = 8;
		break;
	case 0x1b:
		RR(registers[DE], "lo");
		count = 8;
		break;
	case 0x1c:
		RR(registers[HL], "hi");
		count = 8;
		break;
	case 0x1d:
		RR(registers[HL], "lo");
		count = 8;
		break;
	case 0x1e:
		RR();
		count = 16;
		break;
	case 0x1f:
		RR(registers[AF], "hi");
		count = 8;
		break;
	// 2x
	case 0x20:
		SLA(registers[BC], "hi");
		count = 8;
		break;
	case 0x21:
		SLA(registers[BC], "lo");
		count = 8;
		break;
	case 0x22:
		SLA(registers[DE], "hi");
		count = 8;
		break;
	case 0x23:
		SLA(registers[DE], "lo");
		count = 8;
		break;
	case 0x24:
		SLA(registers[HL], "hi");
		count = 8;
		break;
	case 0x25:
		SLA(registers[HL], "lo");
		count = 8;
		break;
	case 0x26:
		SLA();
		count = 16;
		break;
	case 0x27:
		SLA(registers[AF], "hi");
		count = 8;
		break;
	case 0x28:
		SRA(registers[BC], "hi");
		count = 8;
		break;
	case 0x29:
		SRA(registers[BC], "lo");
		count = 8;
		break;
	case 0x2a:
		SRA(registers[DE], "hi");
		count = 8;
		break;
	case 0x2b:
		SRA(registers[DE], "lo");
		count = 8;
		break;
	case 0x2c:
		SRA(registers[HL], "hi");
		count = 8;
		break;
	case 0x2d:
		SRA(registers[HL], "lo");
		count = 8;
		break;
	case 0x2e:
		SRA();
		count = 16;
		break;
	case 0x2f:
		SRA(registers[AF], "hi");
		count = 8;
		break;
	}
	return count;
}

/////////////////////////////////////////////////////////////

// Push data to stack.
void Z80::PUSH(uint16_t data)
{
	sp -= 2;
	WriteMem(sp, data);
}

// Pop data from stack to reg.
void Z80::POP(uint16_t &reg)
{
	reg = ReadMem(sp);
	sp += 2;
}

// Move data to reg(pos can be "hi" or "lo").
void Z80::LD8(uint16_t &reg, std::string pos, uint8_t data)
{
	if(pos == "hi")
	{
		SetHiRegister(reg, data);
	}
	else if(pos == "lo")
	{
		SetLoRegister(reg, data);
	}
	else
	{
		std::cout << "ERROR:LD8::INVALID_POS\n";
	}
}

// Move data to memory in addr.
void Z80::LD8(uint16_t addr, uint8_t data)
{
	WriteMem(addr, data);
}

// Move data to reg.
void Z80::LD16(uint16_t &reg, uint16_t data)
{
	reg = data;
}

// Add data to accumulator. If carry is true, the value in FLAG_C is also added.
void Z80::ADD8(uint8_t data, bool carry)
{
	uint16_t a = GetHiRegister(registers[AF]);
	if(carry)
	{
		data += GetFlag(FLAG_C);
	}
	a += data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, (((a & 0xf) + (data & 0xf)) & 0x10) == 0x10);
	SetFlag(FLAG_C, a > 255);
	SetHiRegister(registers[AF], (uint8_t) a);
}

// Add reg to HL.
void Z80::ADD16(uint16_t reg)
{
	registers[HL] += reg;
}

// Add d(signed 8-bit integer) to SP.
void Z80::ADD16(int8_t d)
{
	sp += d;
}

// Subtract data to accumulator. If carry is true, the value in FLAG_C is also subtracted.
void Z80::SUB(uint8_t data, bool carry)
{
	uint8_t a = GetHiRegister(registers[AF]);
	if(carry)
	{
		data += GetFlag(FLAG_C);
	}
	a -= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, true);
	SetFlag(FLAG_H, ((a & 0xf) - (data & 0xf)) < 0);
	SetFlag(FLAG_C, a < 0);
	SetHiRegister(registers[AF], (uint8_t) a);
}

// Bitwise & data to accumulator.
void Z80::AND(uint8_t data)
{
	uint16_t a = GetHiRegister(registers[AF]);
	a &= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, true);
	SetFlag(FLAG_C, false);
	SetHiRegister(registers[AF], (uint8_t) a);
}

// Bitwise ^ data to accumulator.
void Z80::XOR(uint8_t data)
{
	uint16_t a = GetHiRegister(registers[AF]);
	a ^= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, false);
	SetHiRegister(registers[AF], (uint8_t) a);
}

// Bitwise | data to accumulator.
void Z80::OR(uint8_t data)
{
	uint16_t a = GetHiRegister(registers[AF]);
	a |= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, false);
	SetHiRegister(registers[AF], (uint8_t) a);
}

// Like SUB but accumulator is not modified, only the flags.
void Z80::CP(uint8_t data)
{
	uint8_t a = GetHiRegister(registers[AF]);
	a -= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, true);
	SetFlag(FLAG_H, ((a & 0xf) - (data & 0xf)) < 0);
	SetFlag(FLAG_C, a < 0);
}

// Increments reg(pos can be "hi" or "lo").
void Z80::INC8(uint16_t &reg, std::string pos)
{
	uint16_t r = 0;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		SetHiRegister(reg, ++r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		SetLoRegister(reg, ++r);
	}
	else
	{
		std::cout << "ERROR:INC8::INVALID_POS\n";
	}
}

// Increments the value of memory in addr.
void Z80::INC8(uint16_t addr)
{
	WriteMem(addr, ReadMem(addr) + 1);
}

// Increments reg.
void Z80::INC16(uint16_t &reg)
{
	reg++;
}

// Decrements reg(pos can be "hi" or "lo").
void Z80::DEC8(uint16_t &reg, std::string pos)
{
	uint16_t r = 0;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		SetHiRegister(reg, --r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		SetLoRegister(reg, --r);
	}
	else
	{
		std::cout << "ERROR:DEC8::INVALID_POS\n";
	}
}

// Decrements the value of memory in addr.
void Z80::DEC8(uint16_t addr)
{
	WriteMem(addr, ReadMem(addr) - 1);
}

// Decrements reg.
void Z80::DEC16(uint16_t &reg)
{
	reg--;
}

// Decimal adjust the accumulator.
void Z80::DAA()
{
	uint8_t a = GetHiRegister(registers[AF]);
	uint8_t hn = (a & 0xf0) >> 4;
	uint8_t ln = a & 0x0f;
	if(!GetFlag(FLAG_N))
	{
		if(GetFlag(FLAG_H) || (a & 0xf) > 9)
		{
			a += 0x06;
		}
		if(GetFlag(FLAG_C) || a > 0x9f)
		{
			a += 0x60;
		}
	}
	else
	{
		if(GetFlag(FLAG_H))
		{
			a = (a - 6) & 0xFF;
		}
		if(GetFlag(FLAG_C))
		{
			a -= 0x60;
		}
	}
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, (a & 0x100) == 0x100);
	SetHiRegister(registers[AF], (uint8_t) a);
}

// Bitwise ^ the accumulator to 0xff.
void Z80::CPL()
{
	uint8_t a = GetHiRegister(registers[AF]);
	a ^= 0xff;
	SetFlag(FLAG_N, true);
	SetFlag(FLAG_H, true);
	SetHiRegister(registers[AF], (uint8_t) a);
}

// Rotate accumulator left.
void Z80::RLCA()
{
	uint8_t a = GetHiRegister(registers[AF]);
	uint8_t c = (a & 0x80) >> 7;
	a <<= 1;
	a |= c;
	SetFlag(FLAG_Z, false);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate accumulator left through carry.
void Z80::RLA()
{
	uint8_t a = GetHiRegister(registers[AF]);
	uint8_t c = (a & 0x80) >> 7;
	a <<= 1;
	a |= GetFlag(FLAG_C);
	SetFlag(FLAG_Z, false);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate accumulator right.
void Z80::RRCA()
{
	uint8_t a = GetHiRegister(registers[AF]);
	uint8_t c = (a & 0x01) << 7;
	a >>= 1;
	a |= c;
	SetFlag(FLAG_Z, false);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate accumulator right through carry.
void Z80::RRA()
{
	uint8_t a = GetHiRegister(registers[AF]);
	uint8_t c = (a & 0x01) << 7;
	a >>= 1;
	a |= GetFlag(FLAG_C);
	SetFlag(FLAG_Z, false);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate reg(pos can be "hi" or "lo") left.
void Z80::RLC(uint16_t &reg, std::string pos)
{
	uint8_t r = 0;
	uint8_t c = (r & 0x80) >> 7;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r <<= 1;
		r |= c;
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r <<= 1;
		r |= c;
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:RLC::INVALID_POS\n";
	}
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate the data in memory at (HL) left.
void Z80::RLC()
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t c = (data & 0x80) >> 7;
	data <<= 1;
	data |= c;
	WriteMem(registers[HL], data);
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate reg(pos can be "hi" or "lo") left through carry.
void Z80::RL(uint16_t &reg, std::string pos)
{
	uint8_t r = 0;
	uint8_t c = (r & 0x80) >> 7;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r <<= 1;
		r |= GetFlag(FLAG_C);
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r <<= 1;
		r |= GetFlag(FLAG_C);
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:RL::INVALID_POS\n";
	}
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate the data in memory at (HL) left through carry.
void Z80::RL()
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t c = (data & 0x80) >> 7;
	data <<= 1;
	data |= GetFlag(FLAG_C);
	WriteMem(registers[HL], data);
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate reg(pos can be "hi" or "lo") right.
void Z80::RRC(uint16_t &reg, std::string pos)
{
	uint8_t r = 0;
	uint8_t c = (r & 0x01) << 7;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r >>= 1;
		r |= c;
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r >>= 1;
		r |= c;
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:RRC::INVALID_POS\n";
	}
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate the data in memory at (HL) right.
void Z80::RRC()
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t c = (data & 0x01) << 7;
	data >>= 1;
	data |= c;
	WriteMem(registers[HL], data);
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate reg(pos can be "hi" or "lo") right through carry.
void Z80::RR(uint16_t &reg, std::string pos)
{
	uint8_t r = 0;
	uint8_t c = (r & 0x01) << 7;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r >>= 1;
		r |= GetFlag(FLAG_C);
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r >>= 1;
		r |= GetFlag(FLAG_C);
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:RR::INVALID_POS\n";
	}
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Rotate the data in memory at (HL) right through carry.
void Z80::RR()
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t c = (data & 0x01) << 7;
	data >>= 1;
	data |= GetFlag(FLAG_C);
	WriteMem(registers[HL], data);
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Shift reg(pos can be "hi" or "lo") left.
void Z80::SLA(uint16_t &reg, std::string pos)
{
	uint8_t r = 0;
	uint8_t c = (r & 0x80) >> 7;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r <<= 1;
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r <<= 1;
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:SLA::INVALID_POS\n";
	}
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Shift the data in moemory at (HL) left.
void Z80::SLA()
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t c = (data & 0x80) >> 7;
	data <<= 1;
	WriteMem(registers[HL], data);
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Shift reg(pos can be 'hi" or "lo") left.
void Z80::SRA(uint16_t &reg, std::string pos)
{
	uint8_t r = 0;
	uint8_t c = (r & 0x01) << 7;
	uint8_t b7 = (r & 0x80);
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r >>= 1;
		r |= b7;
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r >>= 1;
		r |= b7;
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:SRA::INVALID_POS\n";
	}
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Shift the data in memory at (HL) right.
void Z80::SRA()
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t c = (data & 0x01) << 7;
	uint8_t b7 = (data & 0x80);
	data >>= 1;
	data |= b7;
	WriteMem(registers[HL], data);
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Swap reg(pos can be "hi" or "lo") low/hi nibble.
void Z80::SWAP(uint16_t &reg, std::string pos)
{
	uint8_t r = 0;
	uint8_t hn = 0;
	uint8_t ln = 0;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		hn = (r & 0xf0) >> 4;
		ln = (r & 0x0f) << 4;
		r = ln | hn;
		SetHiRegister(reg, ln);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		hn = (r & 0xf0) >> 4;
		ln = (r & 0x0f) << 4;
		r = ln | hn;
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:SWAP::INVALID_POS\n";
	}
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, false);
}

// Swap the data in memory at (HL)'s low/hi nibble.
void Z80::SWAP()
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t hn = (data & 0xf0) >> 4;
	uint8_t ln = (data & 0x0f) << 4;
	data = ln | hn;
	WriteMem(registers[HL], data);
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, false);
}

// Shift reg(pos can be "hi" or "lo") right logical.
void Z80::SRL(uint16_t &reg, std::string pos)
{
	uint8_t r = 0;
	uint8_t c = (r & 0x01) << 7;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r >>= 1;
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r >>= 1;
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:SRA::INVALID_POS\n";
	}
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Shift the data in memory at (HL) right logically.
void Z80::SRL()
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t c = (data & 0x01) << 7;
	data >>= 1;
	WriteMem(registers[HL], data);
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

// Test bit n of reg(pos can be "hi" or "lo").
void Z80::BIT(uint16_t &reg, std::string pos, int n)
{
	uint8_t r = 0;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
	}
	else
	{
		std::cout << "ERROR:BIT::INVALID_POS\n";
	}
	uint8_t setter = 1 << n;
	r &= setter;
	r >>= n;
	SetFlag(FLAG_Z, r == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, true);
}

// Test bit n of the data in memory at (HL).
void Z80::BIT(int n)
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t setter = 1 << n;
	data &= setter;
	data >>= n;
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, true);
}

// Set bit n of reg(pos can be "hi" or "lo").
void Z80::SET(uint16_t &reg, std::string pos, int n)
{
	uint8_t r = 0;
	uint8_t setter = 1 << n;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r |= setter;
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r |= setter;
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:SET::INVALID_POS\n";
	}
}

// Set bit n of the data in memory at (HL).
void Z80::SET(int n)
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t setter = 1 << n;
	data |= setter;
	WriteMem(registers[HL], data);
}

// Reset bin n of reg(pos can be "hi" or "lo").
void Z80::RES(uint16_t &reg, std::string pos, int n)
{
	uint8_t r = 0;
	uint8_t setter = 1 << n;
	setter ^= 0xff;
	if(pos == "hi")
	{
		r = GetHiRegister(reg);
		r &= setter;
		SetHiRegister(reg, r);
	}
	else if(pos == "lo")
	{
		r = GetLoRegister(reg);
		r &= setter;
		SetLoRegister(reg, r);
	}
	else
	{
		std::cout << "ERROR:RES::INVALID_POS\n";
	}
}

// Reset bit n of the data in memory at (HL).
void Z80::RES(int n)
{
	uint8_t data = ReadMem(registers[HL]);
	uint8_t setter = 1 << n;
	setter ^= 0xff;
	data &= setter;
	WriteMem(registers[HL], data);
}

// Jump to nn.
void Z80::JP(uint16_t nn)
{
	pc = nn;
}

// Jump to HL.
void Z80::JP()
{
	pc = ReadMem(registers[HL]);
}

// Conditional jump, f can be NZ, Z, NC, C.|
uint8_t Z80::JP(rel_jp_flag f, uint16_t addr)
{
	switch(f)
	{
	case rel_jp_flag::NZ:
		if(!GetFlag(FLAG_Z))
		{
			JP(addr);
			return 4;
		}
		break;
	case rel_jp_flag::Z:
		if(GetFlag(FLAG_Z))
		{
			JP(addr);
			return 4;
		}
		break;
	case rel_jp_flag::NC:
		if(!GetFlag(FLAG_C))
		{
			JP(addr);
			return 4;
		}
		break;
	case rel_jp_flag::C:
		if(GetFlag(FLAG_C))
		{
			JP(addr);
			return 4;
		}
		break;
	default:
		std::cout << "ERROR:JP::INVALID_POS\n";
		return 0;
		break;
	}
}

// Relative jump.
void Z80::JR(int8_t d)
{
	pc += d;
}

// Conditional relative jump, f can be NZ, Z, NC, C.
uint8_t Z80::JR(rel_jp_flag f, int8_t d)
{
	switch(f)
	{
	case rel_jp_flag::NZ:
		if(!GetFlag(FLAG_Z))
		{
			JR(d);
			return 4;
		}
		break;
	case rel_jp_flag::Z:
		if(GetFlag(FLAG_Z))
		{
			JR(d);
			return 4;
		}
		break;
	case rel_jp_flag::NC:
		if(!GetFlag(FLAG_C))
		{
			JR(d);
			return 4;
		}
		break;
	case rel_jp_flag::C:
		if(GetFlag(FLAG_C))
		{
			JR(d);
			return 4;
		}
		break;
	default:
		std::cout << "ERROR:JR::INVALID_POS\n";
		return 0;
	}
}

// Call to nn.
void Z80::CALL(uint16_t nn)
{
	PUSH(pc);
	pc = nn;
}

// Conditional call, f can be NZ, Z, NC, C.
uint8_t Z80::CALL(rel_jp_flag f, uint16_t nn)
{
	switch(f)
	{
	case rel_jp_flag::NZ:
		if(!GetFlag(FLAG_Z))
		{
			CALL(nn);
			return 12;
		}
		break;
	case rel_jp_flag::Z:
		if(GetFlag(FLAG_Z))
		{
			CALL(nn);
			return 12;
		}
		break;
	case rel_jp_flag::NC:
		if(!GetFlag(FLAG_C))
		{
			CALL(nn);
			return 12;
		}
		break;
	case rel_jp_flag::C:
		if(GetFlag(FLAG_C))
		{
			CALL(nn);
			return 12;
		}
		break;
	default:
		std::cout << "ERROR:CALL::INVALID_POS\n";
		return 0;
	}
}

// Return from a subroutine.
void Z80::RET()
{
	POP(pc);
}

// Conditional return, f can be NZ, Z, NC, C.
uint8_t Z80::RET(rel_jp_flag f)
{
	switch(f)
	{
	case rel_jp_flag::NZ:
		if(!GetFlag(FLAG_Z))
		{
			RET();
			return 12;
		}
		break;
	case rel_jp_flag::Z:
		if(GetFlag(FLAG_Z))
		{
			RET();
			return 12;
		}
		break;
	case rel_jp_flag::NC:
		if(!GetFlag(FLAG_C))
		{
			RET();
			return 12;
		}
		break;
	case rel_jp_flag::C:
		if(GetFlag(FLAG_C))
		{
			RET();
			return 12;
		}
		break;
	default:
		std::cout << "ERROR:RET::INVALID_POS\n";
		return 0;
	}
}

// Return from a subroutine and enable interrupts.
void Z80::RETI()
{
	POP(pc);
	IME = true;
}

// Push present address onto stack. Jump to address $0000 + n.
void Z80::RST(uint8_t n)
{
	PUSH(pc);
	JP(0x0000 + n);
}

// Toggles carry flag.
void Z80::CCF()
{
	SetFlag(FLAG_C, GetFlag(FLAG_C) ^ 1);
}

// Sets carry flag.
void Z80::SCF()
{
	SetFlag(FLAG_C, 1);
}

// Halts until interrupt occurs.
void Z80::HALT()
{
	if(IME)
	{
		bool halt = false;
		uint8_t IE = ReadMem(0xff0f);
		for(int i = 0; i < 4; i++)
		{
			uint8_t setter = 1 << i;
			halt = (IE & setter) == 1 ? true : false;
		}
		if(halt)
		{
			pc--;
		}
	}
}

// Standby mode.
void Z80::STOP()
{
	// TODO
}

// disable interrupts, IME = false;
void Z80::DI()
{
	IME = false;
}

// enables interrupts, IME = true;
void Z80::EI()
{
	IME = true;
}

