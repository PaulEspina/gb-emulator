#pragma once

#include <string>
#include <vector>
#include <stdint.h>


typedef int8_t byte; // Signed 8-bits
typedef int16_t word; // Signed 16-bits
typedef uint8_t ubyte; // Unsigned 8-bits
typedef uint16_t uword; // Unsigned 16-bits

constexpr byte AF = 0;
constexpr byte BC = 1;
constexpr byte DE = 2;
constexpr byte HL = 3;

class Z80
{
public:
	Z80();
	bool LoadCartridge(std::string path);
	void Init();
private:
	struct Reg
	{
		byte hi : 4;
		byte lo : 4;
	};
	void Write(word address, byte data);
	byte Read(word address);
	void Cycle();
	byte Fetch();
	byte Decode();
	// OPCODES
	byte LDRR(Reg &l, Reg &r);
	byte LDRN(Reg &reg);
	byte LDHL(Reg &reg, bool imm, bool mode);
	byte LDA(Reg &reg, bool imm, bool mode);
	byte LDH(bool mode);
	byte LDC(bool mode);
	byte LDID(bool mode, bool op);
	byte LD16(Reg &reg);
	byte LDSP();
	byte Push(Reg &reg);
	byte Pop(Reg &reg);
	byte ADDR(Reg &reg, bool carry);
	byte ADDN(bool carry);
	byte ADDHL(bool carry);
	byte SUBR(Reg &reg, bool carry);
	byte SUBN(bool carry);
	byte SUBHL(bool carry);
	byte ANDR(Reg &reg);
	byte ANDN();
	byte ANDHL();
	byte XORR(Reg &reg);
	byte XORN();
	byte XORHL();
	byte ORR(Reg &reg);
	byte ORN();
	byte ORHL();
	byte CPR(Reg &reg);
	byte CPN();
	byte CPHL();
	byte INCR(Reg &reg);
	byte INCHL();
	byte DECR(Reg &reg);
	byte DECHL();
	byte DAA();
	byte CPL();
	byte ADDHLRR(Reg &reg);
	byte ADDSP();
	byte INCRR(Reg &reg);
	byte DECRR(Reg &reg);
	byte LDHLSP();
	byte RLCA();
	byte RLA();
	byte RRCA();
	byte RRA();
	byte RLCR(Reg &reg);
	byte RLCHL();
	byte RLR(Reg &reg);
	byte RLHL();
	byte RRCR(Reg &reg);
	byte RRCHL();
	byte RRR(Reg &reg);
	byte RRHL();
	byte SLAR(Reg &reg);
	byte SLAHL();
	byte SWAPR(Reg &reg);
	byte SWAPHL();
	byte SRAR(Reg &reg);
	byte SRAHL();
	byte SRLR(Reg &reg);
	byte SRLHL();
	byte BITR(Reg &reg);
	byte BITHL();
	byte SETR(Reg &reg);
	byte SETHL();
	byte RESR(Reg &reg);
	byte RESHL();
	byte CCF();
	byte SCF();
	byte NOP();
	byte HALT();
	byte STOP();
	byte DI();
	byte EI();
	byte JPNN();
	byte JPHL();
	byte JPF();
	byte JRPC();
	byte JRFPC();
	byte CALLNN();
	byte CALLF();
	byte RET();
	byte RETF();
	byte RETI();
	byte RST();

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
	Reg registers[4];
	word sp;
	word pc;
	byte cartridge[0x200000];
	byte memory[0x10000];
	byte screen[144][160];
	uword clock_count;
};