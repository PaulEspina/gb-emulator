#include "Z80.h"

#include <fstream>
#include <vector>

Z80::Z80()
{
}

bool Z80::LoadCartridge(std::string path)
{
	rompath = path;
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
		cartridge[i] = (byte) result[i];
	}
	return true;
}

void Z80::Init()
{
	registers[0] = 0x01b0;
	registers[1] = 0x0013;
	registers[2] = 0x00d8;
	registers[4] = 0x014d;
	sp = 0xfffe;
	pc = 0x100;
	memset(&rom, 0, sizeof(rom));
	memset(&ram, 0, sizeof(ram));
	memset(&screen, 0, sizeof(screen));
}
