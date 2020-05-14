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
	uint8_t r = 0;
	uint16_t d = data;
	if(pos == "hi")
	{
		r = GetLoRegister(reg);
		d <<= 8;
	}
	else if(pos == "lo")
	{
		r = GetHiRegister(reg);
	}
	else
	{
		std::cout << "ERROR:LD8::INVALID_POS\n";
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

void Z80::ADD16(uint16_t reg)
{
	registers[HL] += reg;
}

void Z80::ADD16(int8_t d)
{
	sp += d;
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
	SetFlag(FLAG_N, true);
	SetFlag(FLAG_H, ((a & 0xf) - (data & 0xf)) < 0);
	SetFlag(FLAG_C, a < 0);
	SetHiRegister(registers[AF], (uint8_t) a);
}

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

void Z80::CP(uint8_t data)
{
	uint8_t a = GetHiRegister(registers[AF]);
	a -= data;
	SetFlag(FLAG_Z, a == 0);
	SetFlag(FLAG_N, true);
	SetFlag(FLAG_H, ((a & 0xf) - (data & 0xf)) < 0);
	SetFlag(FLAG_C, a < 0);
}

void Z80::INC8(uint8_t &reg, std::string pos)
{
	uint16_t h = GetHiRegister(reg);
	uint8_t l = GetLoRegister(reg);
	if(pos == "hi")
	{
		h++;
	}
	else if(pos == "lo")
	{
		l++;
	}
	else
	{
		std::cout << "ERROR:LD8::INVALID_POS\n";
	}
	h |= l;
	reg = h;
}

void Z80::INC8(uint16_t addr)
{
	WriteMem(addr, ReadMem(addr) + 1);
}

void Z80::INC16(uint16_t &reg)
{
	reg++;
}

void Z80::DEC8(uint8_t &reg, std::string pos)
{
	uint16_t h = GetHiRegister(reg);
	uint8_t l = GetLoRegister(reg);
	if(pos == "hi")
	{
		h--;
	}
	else if(pos == "lo")
	{
		l--;
	}
	else
	{
		std::cout << "ERROR:LD8::INVALID_POS\n";
	}
	h |= l;
	reg = h;
}

void Z80::DEC8(uint16_t addr)
{
	WriteMem(addr, ReadMem(addr) - 1);
}

void Z80::DEC16(uint16_t &reg)
{
	reg--;
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
	SetHiRegister(registers[AF], (uint8_t) a);
}

void Z80::CPL()
{
	uint8_t a = GetHiRegister(registers[AF]);
	a ^= 0xff;
	SetFlag(FLAG_N, true);
	SetFlag(FLAG_H, true);
	SetHiRegister(registers[AF], (uint8_t) a);
}

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

void Z80::RRCA()
{
	uint8_t a = GetHiRegister(registers[AF]);
	uint8_t c = (a & 0x01);
	a >>= 1;
	c <<= 7;
	a |= c;
	SetFlag(FLAG_Z, false);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}

void Z80::RRA()
{
	uint8_t a = GetHiRegister(registers[AF]);
	uint8_t c = (a & 0x01);
	a >>= 1;
	c <<= 7;
	a |= GetFlag(FLAG_C);
	SetFlag(FLAG_Z, false);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}
