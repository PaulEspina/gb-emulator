#pragma once

#include <string>
#include <stdint.h>
#include <vector>
#include <iostream>

constexpr int AF = 0;
constexpr int BC = 1;
constexpr int DE = 2;
constexpr int HL = 3;

constexpr int FLAG_Z = 3;
constexpr int FLAG_N = 2;
constexpr int FLAG_H = 1;
constexpr int FLAG_C = 0;

class Z80
{
public:
	Z80();
	bool LoadCartridge(std::string path);
	void Init();
private:
	uint8_t ReadMem(uint16_t addr);
	void WriteMem(uint16_t addr, uint8_t data);
	uint8_t GetHiRegister(uint16_t reg);
	uint8_t GetLoRegister(uint16_t reg);
	void SetHiRegister(uint16_t &reg, uint8_t data);
	void SetLoRegister(uint16_t &reg, uint8_t data);
	void SetFlag(int bit, bool value);
	bool GetFlag(int bit);
	void Cycle();
	uint8_t Fetch();
	// OPCODES
	// Push data to stack.
	void PUSH(uint16_t data);
	// Pop data from stack to reg.
	void POP(uint16_t &reg);
	// Move data to reg(pos can be "hi" or "lo").
	void LD8(uint16_t &reg, std::string pos, uint8_t data);
	// Move data to memory in addr.
	void LD8(uint16_t addr, uint8_t data);
	// Move data to reg.
	void LD16(uint16_t &rr1, uint16_t data);
	// Add data to accumulator. If carry is true, the value in FLAG_C is also added.
	void ADD8(uint8_t data, bool carry);
	// Add reg to HL.
	void ADD16(uint16_t reg);
	// Add d(signed 8-bit integer) to SP.
	void ADD16(int8_t d);
	// Subtract data to accumulator. If carry is true, the value in FLAG_C is also subtracted.
	void SUB(uint8_t data, bool carry);
	// Bitwise & data to accumulator.
	void AND(uint8_t data);
	// Bitwise ^ data to accumulator.
	void XOR(uint8_t data);
	// Bitwise | data to accumulator.
	void OR(uint8_t data);
	// Like SUB but accumulator is not modified, only the flags.
	void CP(uint8_t data);
	// Increments reg(pos can be "hi" or "lo").
	void INC8(uint8_t &reg, std::string pos);
	// Increments the value of memory in addr.
	void INC8(uint16_t addr);
	// Increments reg.
	void INC16(uint16_t &reg);
	// Decrements reg(pos can be "hi" or "lo").
	void DEC8(uint8_t &reg, std::string pos);
	// Decrements the value of memory in addr.
	void DEC8(uint16_t addr);
	// Decrements reg.
	void DEC16(uint16_t &reg);
	// Decimal adjust the accumulator.
	void DAA();
	// Bitwise ^ the accumulator to 0xff.
	void CPL();
	// Rotate accumulator left.
	void RLCA();
	// Rotate accumulator left through carry.
	void RLA();
	// Rotate accumulator right.
	void RRCA();
	// Rotate accumulator right through carry.
	void RRA();
	/*
		Registers
		0 - AF
		1 - BC
		2 - DE
		3 - HL

		Only the first 4 most significant bit of F is used
		Bit 7(3): Zero flag(Z)
		Bit 6(2): Subtract flag(N)
		Bit 5(1): Half carry flag(H)
		Bit 4(0): Carry flag(C)
	*/
	uint16_t registers[4];
	uint16_t sp;
	uint16_t pc;
	uint8_t cartridge[0x200000];
	uint8_t memory[0x10000];
	uint8_t screen[144][160];
	uint8_t cycle_count;
};