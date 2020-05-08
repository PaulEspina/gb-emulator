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
	registers[3] = 0x014d;
	sp = 0xfffe;
	pc = 0x100;
	memset(&memory, 0, sizeof(memory));
	memset(&screen, 0, sizeof(screen));
}

void Z80::Cycle()
{
	byte opcode = Fetch();
	Decode(opcode);
}

byte Z80::Fetch()
{
	byte opcode = memory[pc];
	pc++;
	return opcode;
}

void Z80::Decode(byte opcode)
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

void Z80::Ex01()
{
	int nn = memory[pc++];
	nn <<= 8;
	nn |= memory[pc++];
	registers[BC] = nn;
}

void Z80::Ex02()
{
	int a = registers[AF];
	a >>= 8;
	memory[registers[BC]] = a;
}

void Z80::Ex03()
{
	registers[BC] += 1;
}

void Z80::Ex04()
{
	int b = registers[BC];
	b >>= 8;
	int c = registers[BC];
	b++;
	b <<= 8;
	b |= c;
	registers[BC] = b;
}

void Z80::Ex05()
{
	int b = registers[BC];
	b >>= 8;
	int c = registers[BC];
	b--;
	b <<= 8;
	b |= c;
	registers[BC] = b;
}