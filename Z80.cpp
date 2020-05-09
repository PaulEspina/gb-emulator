#include "Z80.h"

#include <fstream>

Z80::Z80()
{
	Init();
}

bool Z80::LoadCartridge(std::string path)
{
	std::ifstream file(path, std::ifstream::binary | std::ifstream::in);
	if(file.is_open())
	{
		file.seekg(0, std::ios::end);
		std::streamoff length = file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<char> result((unsigned int) length);
		file.read(&result[0], length);
		for(unsigned int i = 0, size = result.size(); i < size; i++)
		{
			cartridge[i] = (byte) result[i];
		}
		return true;
	}
	return false;
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

void Z80::Write(word address, byte data)
{
	if(address >= 0xa000 && address <= 0x7fff)
	{
		memory[address] = data;
	}
}

byte Z80::Read(word address)
{
	if(address >= 0 && address <= 0xbfff)
	{
		return memory[address];
	}
}

void Z80::Cycle()
{
	if(clock_count == 0)
	{
		byte code = Fetch();

	}
	clock_count--;
}

byte Z80::Fetch()
{
	return Read(pc++);
}

byte Z80::Decode()
{
	return byte();
}

/////////////////////////////////////////////////////////////



