/*
	Copyright (c) 2020 Paul Espina

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

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
	void LoadInfo();
	void Init();
private:
	enum class CartridgeType{ROM = 0, MBC1 = 1, MBC2 = 2, OTHER = 3};
	CartridgeType cartridgeType;
	uint8_t rom_bank, ram_bank;
	bool ram_enabled, rom_ram_mode;
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
	bool IME;
	enum class RelFlag{NZ = 0, Z = 1, NC = 2, C = 3 };
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
	uint8_t Decode(uint8_t opcode);
	uint8_t PrefixCB(uint8_t opcode);
	/* OPCODES */
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
	void INC8(uint16_t &reg, std::string pos);
	// Increments the value of memory in addr.
	void INC8(uint16_t addr);
	// Increments reg.
	void INC16(uint16_t &reg);
	// Decrements reg(pos can be "hi" or "lo").
	void DEC8(uint16_t &reg, std::string pos);
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
	/* PREFIX CB OPCODES */
	// Rotate reg(pos can be "hi" or "lo") left.
	void RLC(uint16_t &reg, std::string pos);
	// Rotate the data in memory at (HL) left.
	void RLC();
	// Rotate reg(pos can be "hi" or "lo") left through carry.
	void RL(uint16_t &reg, std::string pos);
	// Rotate the data in memory at (HL) left through carry.
	void RL();
	// Rotate reg(pos can be "hi" or "lo") right.
	void RRC(uint16_t &reg, std::string pos);
	// Rotate the data in memory at (HL) right.
	void RRC();
	// Rotate reg(pos can be "hi" or "lo") right through carry.
	void RR(uint16_t &reg, std::string pos);
	// Rotate the data in memory at (HL) right through carry.
	void RR();
	// Shift reg(pos can be "hi" or "lo") left.
	void SLA(uint16_t &reg, std::string pos);
	// Shift the data in memory at (HL) left.
	void SLA();
	// Shift reg(pos can be 'hi" or "lo") right.
	void SRA(uint16_t &reg, std::string pos);
	// Shift the data in memory at (HL) right.
	void SRA();
	// Swap reg(pos can be "hi" or "lo")'s low/hi nibble.
	void SWAP(uint16_t &reg, std::string pos);
	// Swap the data in memory at (HL)'s low/hi nibble.
	void SWAP();
	// Shift reg(pos can be "hi" or "lo") right logically.
	void SRL(uint16_t &reg, std::string pos);
	// Shift the data in memory at (HL) right logically.
	void SRL();
	// Test bit n of reg(pos can be "hi" or "lo").
	void BIT(uint16_t &reg, std::string pos, int n);
	// Test bit n of the data in memory at (HL).
	void BIT(int n);
	// Set bit n of reg(pos can be "hi" or "lo").
	void SET(uint16_t &reg, std::string pos, int n);
	// Set bit n of the data in memory at (HL).
	void SET(int n);
	// Reset bin n of reg(pos can be "hi" or "lo").
	void RES(uint16_t &reg, std::string pos, int n);
	// Reset bit n of the data in memory at (HL).
	void RES(int n);
	// Jump to nn.
	void JP(uint16_t nn);
	// Jump to HL.
	void JP();
	// Conditional jump, f can be NZ, Z, NC, C.
	uint8_t JP(RelFlag f, uint16_t addr);
	// Relative jump.
	void JR(int8_t d);
	// Conditional relative jump, f can be NZ, Z, NC, C.
	uint8_t JR(RelFlag f, int8_t d);
	// Call to nn.
	void CALL(uint16_t nn);
	// Conditional call, f can be NZ, Z, NC, C.
	uint8_t CALL(RelFlag, uint16_t nn);
	// Return from a subroutine.
	void RET();
	// Conditional return, f can be NZ, Z, NC, C.
	uint8_t RET(RelFlag f);
	// Return from a subroutine and enable interrupts.
	void RETI();
	// Push present address onto stack. Jump to address $0000 + n.
	void RST(uint8_t n);
	// Toggles carry flag.
	void CCF();
	// Sets carry flag.
	void SCF();
	// Halts until interrupt occurs.
	void HALT();
	// Standby mode.
	void STOP();
	// disable interrupts, IME = false;
	void DI();
	// enables interrupts, IME = true;
	void EI();
};