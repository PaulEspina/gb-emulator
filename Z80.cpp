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

uint8_t Z80::GetHiRegister(uint16_t reg)
{
	return reg >> 8;
}

uint8_t Z80::GetLoRegister(uint16_t reg)
{
	return reg & 0x00ff;
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

// regl = regr
void Z80::LD8(uint16_t &r, std::string pos, uint8_t data)
{
	uint16_t data;
	if(pos == "hi")
	{
		r &= 0x00ff;
		data <<= 8;
	}
	else if(pos == "lo")
	{
		r &= 0xff00;
	}
	r |= data;
}

void Z80::LD8(uint16_t addr, uint8_t data)
{
	WriteMem(addr, data);
}
