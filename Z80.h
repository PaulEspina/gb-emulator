#pragma once

#include <string>
#include <stdint.h>

constexpr int AF = 0;
constexpr int BC = 1;
constexpr int DE = 2;
constexpr int HL = 3;

class Z80
{
public:
	Z80();
	bool LoadCartridge(std::string path);
	void Init();
	uint8_t GetHiRegister(uint16_t reg);
	uint8_t GetLoRegister(uint16_t reg);
private:
	void SetHiRegister(uint16_t &reg, uint8_t hi);
	void SetLoRegister(uint16_t &reg, uint8_t lo);
	void Cycle();
	uint8_t Fetch();
	void Decode(uint8_t opcode);
	// OPCODES
	
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
	uint16_t registers[4];
	uint16_t sp;
	uint16_t pc;
	uint8_t cartridge[0x200000];
	uint8_t memory[0x10000];
	uint8_t screen[144][160];
};