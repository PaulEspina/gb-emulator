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

// Push data to stack.
void Z80::PUSH(uint16_t data)
{
	WriteMem(sp--, data);
}

// Pop data from stack to reg.
void Z80::POP(uint16_t &reg)
{
	reg = ReadMem(--sp);
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
	uint8_t data = memory[registers[HL]];
	uint8_t c = (data & 0x80) >> 7;
	data <<= 1;
	data |= c;
	memory[registers[HL]] = data;
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
	uint8_t data = memory[registers[HL]];
	uint8_t c = (data & 0x80) >> 7;
	data <<= 1;
	data |= GetFlag(FLAG_C);
	memory[registers[HL]] = data;
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
	uint8_t data = memory[registers[HL]];
	uint8_t c = (data & 0x01) << 7;
	data >>= 1;
	data |= c;
	memory[registers[HL]] = data;
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
	uint8_t data = memory[registers[HL]];
	uint8_t c = (data & 0x01) << 7;
	data >>= 1;
	data |= GetFlag(FLAG_C);
	memory[registers[HL]] = data;
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
	uint8_t data = memory[registers[HL]];
	uint8_t c = (data & 0x80) >> 7;
	data <<= 1;
	memory[registers[HL]] = data;
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

// Shift the data in memory at (HL) right.
void Z80::SRA()
{
	uint8_t data = memory[registers[HL]];
	uint8_t c = (data & 0x01) << 7;
	data >>= 1;
	memory[registers[HL]] = data;
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
	uint8_t data = memory[registers[HL]];
	uint8_t hn = (data & 0xf0) >> 4;
	uint8_t ln = (data & 0x0f) << 4;
	data = ln | hn;
	memory[registers[HL]] = data;
	SetFlag(FLAG_Z, data == 0);
	SetFlag(FLAG_N, false);
	SetFlag(FLAG_H, false);
	SetFlag(FLAG_C, c);
}
