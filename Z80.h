#pragma once

#include <string>
#include <stdint.h>

typedef int8_t byte; // Signed 8-bits
typedef int16_t word; // Signed 16-bits
typedef uint8_t ubyte; // Unsigned 8-bits
typedef uint16_t uword; // Unsigned 16-bits

class Z80
{
public:
	Z80();
	bool LoadCartridge(std::string path);
	void Init();
private:
	/*
		Registers
		0 - AF
		1 - BC
		2 - DE
		3 - HL

		Only the first 4 most significant bit of F is used
		Bit 7: Zero flag(Z)
		Bit 6: Subtract flag(N)
		Bit 5: Half carry flag(H)
		Bit 4: Carry flag(C)
	*/
	word registers[4];
	word sp;
	word pc;
	byte cartridge[0x200000];
	byte rom[0x10000];
	byte ram[0x7d00];
	byte screen[144][160][3];
	std::string rompath;
}

