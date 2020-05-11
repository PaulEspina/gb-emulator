#pragma once

#include <string>
#include <stdint.h>
#include <vector>

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
	uint8_t ReadMem(uint16_t addr);
	void WriteMem(uint16_t addr, uint8_t data);
	void SetHiRegister(uint16_t &reg, uint8_t hi);
	void SetLoRegister(uint16_t &reg, uint8_t lo);
	void Cycle();
	uint8_t Fetch();
	// OPCODES
	/*/////////*/void Ex01(); void Ex02(); void Ex03(); void Ex04(); void Ex05(); void Ex06(); void Ex07(); void Ex08(); void Ex09(); void Ex0A(); void Ex0B(); void Ex0C(); void Ex0D(); void Ex0E(); void Ex0F();
	void Ex10(); void Ex11(); void Ex12(); void Ex13(); void Ex14(); void Ex15(); void Ex16(); void Ex17(); void Ex18(); void Ex19(); void Ex1A(); void Ex1B(); void Ex1C(); void Ex1D(); void Ex1E(); void Ex1F();
	void Ex20(); void Ex21(); void Ex22(); void Ex23(); void Ex24(); void Ex25(); void Ex26(); void Ex27(); void Ex28(); void Ex29(); void Ex2A(); void Ex2B(); void Ex2C(); void Ex2D(); void Ex2E(); void Ex2F();
	void Ex30(); void Ex31(); void Ex32(); void Ex33(); void Ex34(); void Ex35(); void Ex36(); void Ex37(); void Ex38(); void Ex39(); void Ex3A(); void Ex3B(); void Ex3C(); void Ex3D(); void Ex3E(); void Ex3F();
	void Ex40(); void Ex41(); void Ex42(); void Ex43(); void Ex44(); void Ex45(); void Ex46(); void Ex47(); void Ex48(); void Ex49(); void Ex4A(); void Ex4B(); void Ex4C(); void Ex4D(); void Ex4E(); void Ex4F();
	void Ex50(); void Ex51(); void Ex52(); void Ex53(); void Ex54(); void Ex55(); void Ex56(); void Ex57(); void Ex58(); void Ex59(); void Ex5A(); void Ex5B(); void Ex5C(); void Ex5D(); void Ex5E(); void Ex5F();
	void Ex60(); void Ex61(); void Ex62(); void Ex63(); void Ex64(); void Ex65(); void Ex66(); void Ex67(); void Ex68(); void Ex69(); void Ex6A(); void Ex6B(); void Ex6C(); void Ex6D(); void Ex6E(); void Ex6F();
	void Ex70(); void Ex71(); void Ex72(); void Ex73(); void Ex74(); void Ex75(); void Ex76(); void Ex77(); void Ex78(); void Ex79(); void Ex7A(); void Ex7B(); void Ex7C(); void Ex7D(); void Ex7E(); void Ex7F();
	void Ex80(); void Ex81(); void Ex82(); void Ex83(); void Ex84(); void Ex85(); void Ex86(); void Ex87(); void Ex88(); void Ex89(); void Ex8A(); void Ex8B(); void Ex8C(); void Ex8D(); void Ex8E(); void Ex8F();
	void Ex90(); void Ex91(); void Ex92(); void Ex93(); void Ex94(); void Ex95(); void Ex96(); void Ex97(); void Ex98(); void Ex99(); void Ex9A(); void Ex9B(); void Ex9C(); void Ex9D(); void Ex9E(); void Ex9F();
	void ExA0(); void ExA1(); void ExA2(); void ExA3(); void ExA4(); void ExA5(); void ExA6(); void ExA7(); void ExA8(); void ExA9(); void ExAA(); void ExAB(); void ExAC(); void ExAD(); void ExAE(); void ExAF();
	void ExB0(); void ExB1(); void ExB2(); void ExB3(); void ExB4(); void ExB5(); void ExB6(); void ExB7(); void ExB8(); void ExB9(); void ExBA(); void ExBB(); void ExBC(); void ExBD(); void ExBE(); void ExBF();
	void ExC0(); void ExC1(); void ExC2(); void ExC3(); void ExC4(); void ExC5(); void ExC6(); void ExC7(); void ExC8(); void ExC9(); void ExCA(); void ExCB(); void ExCC(); void ExCD(); void ExCE(); void ExCF();
	void ExD0(); void ExD1(); void ExD2();/*/////////*/ void ExD4(); void ExD5(); void ExD6(); void ExD7(); void ExD8(); void ExD9(); void ExDA();/*/////////*/ void ExDC();/*/////////*/ void ExDE(); void ExDF();
	void ExE0(); void ExE1(); void ExE2();/*/////////*//*/////////*/ void ExE5(); void ExE6(); void ExE7(); void ExE8(); void ExE9(); void ExEA();/*/////////*//*/////////*//*/////////*/ void ExEE(); void ExEF();
	void ExF0(); void ExF1(); void ExF2(); void ExF3();/*/////////*/ void ExF5(); void ExF6(); void ExF7(); void ExF8(); void ExF9(); void ExFA(); void ExFB();/*/////////*//*/////////*/ void ExFE(); void ExFF();
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
	uint8_t cycle_count;
};