#include "Z80.h"

#include <fstream>
#include <vector>

Z80::Z80()
{
}

bool Z80::LoadCartridge(std::string path)
{
	std::ifstream file(path, std::ifstream::binary | std::ifstream::in);
	if(!file.is_open())
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
	return reg >>= 8;
}

uint8_t Z80::GetLoRegister(uint16_t reg)
{
	return reg & 0x00ff;
}

void Z80::Cycle()
{
	uint8_t opcode = Fetch();
	Decode(opcode);
}

uint8_t Z80::Fetch()
{
	uint8_t opcode = memory[pc];
	pc++;
	return opcode;
}

void Z80::Decode(uint8_t opcode)
{
	if(opcode == 0xCB)
	{
		opcode = Fetch();
		switch(opcode)
		{

		}
	}
	else
	{
		switch(opcode)
		{
			break;
		}
	}
}

/////////////////////////////////////////////////////////////
