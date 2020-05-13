#include "Z80.h"

#include <fstream>

Z80::Z80()
{
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
	uint8_t setter = 4 << bit;
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
	uint8_t setter = 4 << bit;
	return f & setter;
}

void Z80::Cycle()
{
	if(cycle_count > 0)
	{
		uint8_t opcode = Fetch();

	}
	cycle_count--;
}

uint8_t Z80::Fetch()
{
	uint8_t opcode = memory[pc];
	pc++;
	return opcode;
}

/////////////////////////////////////////////////////////////

void Z80::PUSH(uint16_t data)
{
	WriteMem(sp--, data);
}

void Z80::POP(uint16_t &reg)
{
	reg = ReadMem(--sp);
}

void Z80::LD8(uint16_t &reg, std::string pos, uint8_t data)
{
	uint8_t r = GetHiRegister(reg);
	uint16_t d = data;
	if(pos == "hi")
	{
		r = GetLoRegister(reg);
		d <<= 8;
	}
	r |= d;
	reg = r;
}

void Z80::LD8(uint16_t addr, uint8_t data)
{
	WriteMem(addr, data);
}

void Z80::LD16(uint16_t &reg, uint16_t data)
{
	reg = data;
}

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

void Z80::SUB(uint8_t data, bool carry)
{
	uint8_t a = GetHiRegister(registers[AF]);
	if(carry)
	{
		data += GetFlag(FLAG_C);
	}
	a -= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, ((a & 0xf) - (data & 0xf)) < 0);
	SetFlag(FLAG_C, a < 0);
	SetHiRegister(registers[AF], (uint8_t) a);
}

void Z80::AND(uint8_t data)
{
	uint16_t a = GetHiRegister(registers[AF]);
	a &= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 1);
	SetFlag(FLAG_C, 0);
	SetHiRegister(registers[AF], (uint8_t) a);
}

void Z80::XOR(uint8_t data)
{
	uint16_t a = GetHiRegister(registers[AF]);
	a ^= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_C, 0);
	SetHiRegister(registers[AF], (uint8_t) a);
}

void Z80::OR(uint8_t data)
{
	uint16_t a = GetHiRegister(registers[AF]);
	a |= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_C, 0);
	SetHiRegister(registers[AF], (uint8_t) a);
}

void Z80::CP(uint8_t data)
{
	uint8_t a = GetHiRegister(registers[AF]);
	a -= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, ((a & 0xf) - (data & 0xf)) < 0);
	SetFlag(FLAG_C, a < 0);
}

void Z80::INC(uint8_t &reg)
{
	reg++;
}

void Z80::INC(uint16_t addr)
{
	WriteMem(addr, ReadMem(addr) + 1);
}

void Z80::DEC(uint8_t &reg)
{
	reg--;
}

void Z80::DEC(uint16_t addr)
{
	WriteMem(addr, ReadMem(addr) - 1);
}


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
	SetHiRegister(registers[AF], (uint8_t) a)
}

