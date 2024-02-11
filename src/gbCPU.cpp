#include "gbCPU.h"
#include <fstream>
#include <iostream>
std::ofstream myfile;

#define ZMASK 0b10000000
#define NMASK 0b01000000
#define HMASK 0b00100000
#define CMASK 0b00010000

gbCPU::gbCPU(gbMEM* memory) {
    MEM = memory;
	dMEM = memory->MEM;
    initCpu();
}

uint8_t gbCPU::instruction(){
	#ifdef DEBUG
		if(dMEM[0xFFFF] & 0b00000010){
			// printInstruction();
			// printf("stat wanted\n");
		}
	#endif
	if(halted){
		return 4;
	}
	switch (dMEM[registers.pc]) {
	case 0x00:      //NOP
	{
		registers.pc++;
		return 4;
	}
	case 0x01:		//LD BC, nn
	{
		//load nn into BC
		registers.pc++;
		registers.bc = dMEM[registers.pc] + (dMEM[registers.pc + 1] << 8);
		registers.pc += 2;//count past the two parameters
		return 12;
	}
	case 0x02:		//LD (BC), A
	{
		//Put A into MEM at BC
		MEM->write(registers.bc, registers.a);
		registers.pc++;
		return 8;
	}
	case 0x03:      //INC BC
	{
		//Increments Register BC by 1
		registers.bc++;
		registers.pc++;
		return 8;
	}
	case 0x04:		//INC B
	{
		//increment register B
		registers.b++;
		registers.pc++;
		setN(0);
		setH(registers.b % 0x0F == 0);
		setZ(!registers.b);
		return 4;
	}
	case 0x05:      //DEC B
	{
		//Decrement Register B
		setN(1);
		setH(!(registers.b & 0x0F));
		registers.b--;
		setZ(!registers.b);
		registers.pc++;
		return 4;
	}
	case 0x06:      //LD B, n
	{
		//load n into b
		registers.pc++;//count past param
		registers.b = dMEM[registers.pc];
		registers.pc++;
		return 8;
	}
	case 0x07:		//RLCA
	{
		//Rotate A (circular) left. old bit 7 into C;
		registers.pc++;
		setC(registers.a & 0x80);
		registers.a = (registers.a & 0x7F) << 1;
		registers.a += (registers.f & 0b00010000) >> 4;
		setZ(0);
		setN(0);
		setH(0);
		return 4;
	}
	case 0x08:		//LD (nn),SP
	{
		//Put SP into MEM at adress nn
		registers.pc++;
		MEM->write(dMEM[registers.pc] + (dMEM[registers.pc + 1] << 8),      registers.sp & 0x00FF      );
		MEM->write(dMEM[registers.pc] + (dMEM[registers.pc + 1] << 8) + 1, (registers.sp & 0xFF00 >> 8));
		registers.pc++;//count past the two parameters
		registers.pc++;
		return 20;
	}
	case 0x09:		//ADD HL, BC
	{
		//Add BC to HL
		setN(0);
		setC(((int)registers.bc + (int)registers.hl) > 65535);
		setH(((registers.bc & 0x0FFF) + (registers.hl & 0x0FFF)) > 4095);
		registers.hl = registers.hl + registers.bc;
		registers.pc++;
		return 8;
	}
	case 0x0A:		//LD A, (BC)
	{
		//Load value at adress (BC) into A
		registers.a = dMEM[registers.bc];
		registers.pc++;
		return 8;
	}
	case 0x0B:		//DEC BC
	{
		//decrement register BC
		registers.bc--;
		registers.pc++;
		return 8;
	}
	case 0x0C:		//INC C
	{
		//increment register C
		registers.pc++;
		registers.c++;
		setN(0);
		setZ(!registers.c);
		setH(registers.c % 16 == 0);
		return 4;
	}
	case 0x0D:      //DEC C
	{
		//Decrement Register c
		setH(!(registers.c & 0x0F));
		registers.c--;
		setZ(!registers.c);
		setN(1);
		registers.pc++;
		return 4;
	}
	case 0x0E:      //LD C, n
	{
		//load n into C
		registers.pc++;//count past param
		registers.c = dMEM[registers.pc];
		registers.pc++;
		return 8;
	}
	case 0x0F:		//RRCA
	{
		//Rotate A Right, old bit 0 into carry flag
		setN(0);
		setH(0);
		uint8_t c = registers.a & 0b00000001;
		setC(registers.a & 0b00000001);
		registers.a = (registers.a & 0xFF) >> 1;
		registers.a += (c << 7);
		setZ(0);
		registers.pc++;
		return 4;
	}
	case 0x10:		//STOP
	{
		//Wait for Button Press
		Failure(1);
		registers.pc++;
		return 1;
	}
	case 0x11:		//LD DE, nn
	{
		//load nn into register DE
		registers.pc++;
		registers.de = (dMEM[registers.pc] + (dMEM[registers.pc + 1] << 8));
		registers.pc++;//count past the two parameters
		registers.pc++;
		return 12;
	}
	case 0x12:		//LD (DE), A
	{
		//load A into the adress (DE)
		MEM->write(registers.de, registers.a);
		registers.pc++;
		return 12;
	}
	case 0x13:		//INC DE
	{
		//increment register DE
		registers.de++;
		registers.pc++;
		return 8;
	}
	case 0x14:		//INC D
	{
		//increment register D
		registers.d++;
		setN(0);
		setZ(!registers.d);
		setH(registers.d % 16 == 0);
		registers.pc++;
		return 4;
	}
	case 0x15:		//DEC D
	{
		//Decrement Register D
		setH(!(registers.d & 0x0F));
		registers.d--;
		setZ(!registers.d);
		setN(1);
		registers.pc++;
		return 4;
	}
	case 0x16:		//LD D, n
	{
		//load n into D
		registers.pc++;
		registers.d = dMEM[registers.pc];
		registers.pc++;//count past param
		return 8;
	}
	case 0x17:		//RLA
	{
		//Rotate A left through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.a & 0x80);
		registers.a = (registers.a & 0x7F) << 1;
		registers.a += n;
		setZ(0);
		registers.pc++;
		return 4;
	}
	case 0x18:		//JR n
	{
		//jump to current adress + n (signed)
		registers.pc++;
		registers.pc = registers.pc + ((signed char)dMEM[registers.pc]);
		registers.pc++;
		return 12;
	}
	case 0x19:		//ADD HL, DE
	{
		//Add DE to HL
		setN(0);
		setC(((int)registers.de + (int)registers.hl) > 65535);
		setH(((registers.de & 0x0FFF) + (registers.hl & 0x0FFF)) > 4095);
		registers.hl = registers.hl + registers.de;
		registers.pc++;
		return 8;
	}
	case 0x1A:		//LD A, (DE)
	{
		//Load value at (DE) into A
		registers.a = dMEM[registers.de];
		registers.pc++;
		return 8;
	}
	case 0x1B:		//DEC DE
	{
		//decrement register DE
		registers.de--;
		registers.pc++;
		return 8;
	}
	case 0x1C:		//INC E
	{
		//increment register E
		registers.e++;
		setN(0);
		setH(registers.e % 16 == 0);
		setZ(!registers.e);
		registers.pc++;
		return 4;
	}
	case 0x1D:		//DEC E
	{
		//Decrement Register E
		setH(!(registers.e & 0x0F));
		registers.e--;
		setZ(!registers.e);
		setN(1);
		registers.pc++;
		return 4;
	}
	case 0x1E:		//LD E, n
	{
		//load n into E
		registers.pc++;
		registers.e = dMEM[registers.pc];
		registers.pc++;//count past param
		return 8;
	}
	case 0x1F:		//RRA
	{
		//Rotate A Right through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.a & 0x01);
		registers.a = (registers.a & 0xFE) >> 1;
		registers.a += ((int)n) << 7;
		setZ(0);
		registers.pc++;
		return 4;
	}
	case 0x20:      //JR NZ, n
	{
		//jump to current address plus n if Zflag is reset 
		if (!(registers.f & 0b10000000)) {
			registers.pc++;
			registers.pc = registers.pc + ((signed char)dMEM[registers.pc]); //forces twos complement and adjusts for counting past instruction
			registers.pc++;
			return 12;
		}
		else {
			registers.pc++;//jump past parameter
			registers.pc++;
			return 8;
		}
	}
	case 0x21:      //LD HL, nn
	{
		//put value nn into HL, LSByte first
		registers.pc++;
		registers.hl = (dMEM[registers.pc] + (dMEM[registers.pc + 1] << 8));
		registers.pc++;
		registers.pc++;//count past the two parameters
		return 12;
	}
	case 0x22:		//LDI (HL), A
	{
		//put MEM in A into MEM adress HL, increment HL.
		MEM->write(registers.hl, registers.a);
		registers.hl++;
		registers.pc++;
		return 8;
	}
	case 0x23:		//INC HL
	{
		//increment register HL
		registers.hl++;
		registers.pc++;
		return 8;
	}
	case 0x24:		//INC H
	{
		//increment register H
		registers.h++;
		setN(0);
		setH(registers.h % 16 == 0);
		setZ(!registers.h);
		registers.pc++;
		return 4;
	}
	case 0x25:		//DEC H
	{
		//Decrement Register H
		setH(!(registers.h & 0x0F));
		registers.h--;
		setZ(!registers.h);
		setN(1);
		registers.pc++;
		return 4;
	}
	case 0x26:      //LD H, n
	{
		//load n into H
		registers.pc++;
		registers.h = dMEM[registers.pc];
		registers.pc++;//count past param
		return 8;
	}
	case 0x27:		//DAA
	{
		//Decimal adjust register A
		// Failure(0);
		//TODO:: needs testing
		if (!(registers.f & 0b01000000)) {  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
			if ((registers.f & 0b00010000) || (registers.a > 0x99)) { registers.a += 0x60; setC(1); }
			if ((registers.f & 0b00100000) || ((registers.a & 0x0f) > 0x09)) { registers.a += 0x6; }
		}
		else {  // after a subtraction, only adjust if (half-)carry occurred
			if ((registers.f & 0b00010000)) { registers.a -= 0x60; }
			if ((registers.f & 0b00100000)) { registers.a -= 0x6; }
		}
		// these flags are always updated
		setZ(!(registers.a)); // the usual z flag
		setH(0); // h flag is always cleared
		registers.pc++;
		return 4;
	}
	case 0x28:		//JR Z, n
	{
		//jump to current address plus n if Zflag is set 
		if (registers.f & 0b10000000) {
			registers.pc++;
			registers.pc = registers.pc + ((signed char)dMEM[registers.pc]); //forces twos complement and adjusts for counting past instruction
			registers.pc++;
			return 12;
		}
		else {
			registers.pc++;//jump past parameter
			registers.pc++;
			return 8;
		}
		return 4;
	}
	case 0x29:		//ADD HL, HL
	{
		//Add HL to HL
		setN(0);
		setC(((int)registers.hl + (int)registers.hl) > 65535);
		setH(((registers.hl & 0x0FFF) + (registers.hl & 0x0FFF)) > 4095);
		registers.hl = registers.hl + registers.hl;
		registers.pc++;
		return 8;
	}
	case 0x2A:		//LD A, (HL+)
	{
		//put value at Adress HL into A and increment HL;
		registers.a = MEM->read(registers.hl);
		registers.hl++;
		registers.pc++;
		return 8;
	}
	case 0x2B:		//DEC HL
	{
		//decrement register HL
		registers.hl--;
		registers.pc++;
		return 8;
	}
	case 0x2C:		//INC L
	{
		//increment register L
		registers.l++;
		setN(0);
		setZ(!registers.l);
		setH(registers.l % 16 == 0);
		registers.pc++;
		return 4;
	}
	case 0x2D:		//DEC L
	{
		//Decrement Register L
		setH(!(registers.l & 0x0F));
		registers.l--;
		setZ(!registers.l);
		setN(1);
		registers.pc++;
		return 4;
	}
	case 0x2E:		//LD L, n
	{
		//load n into L
		registers.pc++;
		registers.l = dMEM[registers.pc];
		registers.pc++;//count past param
		return 8;
	}
	case 0x2F:		//CPL
	{
		//Complement A register (Flip all bits)
		registers.f |= 0b01100000; //set N and H flags
		registers.a = (-registers.a) - 1; //two's complement shenanigans
		registers.pc++;
		return 4;
	}
	case 0x30:		//JR NC, n
	{
		//jump to current address plus n if Cflag is reset 
		if (!(registers.f & 0b00010000)) {
			registers.pc++;
			registers.pc = registers.pc + ((signed char)dMEM[registers.pc]); //forces twos complement and adjusts for counting past instruction
			registers.pc++;
			return 12;
		}
		else {
			registers.pc++;//jump past parameter
			registers.pc++;
			return 8;
		}
		return 4;
	}
	case 0x31:		//LD SP, nn
	{
		//set stack pointer to nn
		registers.pc++;
		registers.sp = dMEM[registers.pc] + (dMEM[registers.pc + 1] << 8);
		registers.pc++;
		registers.pc++;//count past the two parameters
		return 12;
	}
	case 0x32:      //LDD (HL), A
	{
		//load data(decrement) from A into (HL)
		MEM->write(registers.hl, registers.a);
		registers.hl--;
		registers.pc++;
		return 8;
	}
	case 0x33:		//INC SP
	{
		//increment register SP
		registers.sp++;
		registers.pc++;
		return 8;
	}
	case 0x34:		//INC (HL)
	{
		//Increment data at adress HL
		uint8_t hlData = MEM->read(registers.hl);
		hlData++;
		setZ(!hlData);
		setH(hlData % 16 == 0);
		setN(0);
		MEM->write(registers.hl, hlData);
		registers.pc++;
		return 12;
	}
	case 0x35:		//DEC (HL)
	{
		//Decrement data at adress HL
		uint8_t hlData = MEM->read(registers.hl);
		setH(!(hlData & 0x0F));
		hlData--;
		setZ(!hlData);
		setN(1);
		MEM->write(registers.hl, hlData);
		registers.pc++;
		return 12;
	}
	case 0x36:		//LD (HL), n
	{
		//Load n into MEM at (HL)
		registers.pc++;
		MEM->write(registers.hl, dMEM[registers.pc]);
		registers.pc++;//count past param
		return 12;
	}
	case 0x37:		//SCF
	{
		//Set Carry flag
		setN(0);
		setH(0);
		setC(1);
		registers.pc++;
		return 4;
	}
	case 0x38:		//JR C, n
	{
		//jump to current address plus n if Cflag is set 
		if (registers.f & CMASK) {
			registers.pc++;
			registers.pc = registers.pc + ((signed char)dMEM[registers.pc]); //forces twos complement and adjusts for counting past instruction
			registers.pc++;
			return 12;
		}
		else {
			registers.pc++;//jump past parameter
			registers.pc++;
			return 8;
		}
		return 4;
	}
	case 0x39:		//ADD HL, SP
	{
		//Add SP to HL
		setN(0);
		setC(((int)registers.sp + (int)registers.hl) > 65535);
		setH(((registers.sp & 0x0FFF) + (registers.hl & 0x0FFF)) > 4095);
		registers.hl = registers.hl + registers.sp;
		registers.pc++;
		return 8;
	}
	case 0x3A:		//LD A, (HL-)
	{
		//put value at Adress HL into A and decrement HL;
		registers.a = MEM->read(registers.hl);
		registers.hl--;
		registers.pc++;
		return 8;
	}
	case 0x3B:		//DEC SP
	{
		//decrement Stack Pointer
		registers.sp--;
		registers.pc++;
		return 8;
	}
	case 0x3C:		//INC A
	{
		//Increment Register A
		registers.a++;
		setZ(!registers.a);
		setH(registers.a % 16 == 0);
		setN(0);
		registers.pc++;
		return 4;
	}
	case 0x3D:		//DEC A
	{
		//Decrement Register A
		setH(!(registers.a & 0x0F));
		registers.a--;
		setZ(!registers.a);
		setN(1);
		registers.pc++;
		return 4;
	}
	case 0x3E:      //LD A, n
	{
		//load n into A
		registers.pc++;
		registers.a = dMEM[registers.pc];
		registers.pc++;//count past param
		return 8;
	}
	case 0x3F:		//CCF
	{
		//Complement Carry Flag
		setN(0);
		setH(0);
		setC(!(registers.f & 0b00010000));
		registers.pc++;
		return 4;
	}
	case 0x40:		//LD B, B
	{
		//Put value of register B into register B
		//registers.b = registers.b;
		registers.pc++;
		return 4;
	}
	case 0x41:		//LD B, C
	{
		//Put value of register C into register B
		registers.b = registers.c;
		registers.pc++;
		return 4;
	}
	case 0x42:		//LD B, D
	{
		//Put value of register D into register B
		registers.b = registers.d;
		registers.pc++;
		return 4;
	}
	case 0x43:		//LD B, E
	{
		//Put value of register E into register B
		registers.b = registers.e;
		registers.pc++;
		return 4;
	}
	case 0x44:		//LD B, H
	{
		//Put value of register H into register B
		registers.b = registers.h;
		registers.pc++;
		return 4;
	}
	case 0x45:		//LD B, L
	{
		//Put value of register L into register B
		registers.b = registers.l;
		registers.pc++;
		return 4;
	}
	case 0x46:		//LD B, (HL)
	{
		//put value at adress HL into register B
		registers.b = MEM->read(registers.hl);
		registers.pc++;
		return 8;
	}
	case 0x47:		//LD B, A
	{
		//Put value of register A into register B
		registers.b = registers.a;
		registers.pc++;
		return 4;
	}
	case 0x48:		//LD C, B
	{
		//Put value of register B into register C
		registers.c = registers.b;
		registers.pc++;
		return 4;
	}
	case 0x49:		//LD C, C
	{
		//Put value of register C into register C
		//registers.c = registers.c;
		registers.pc++;
		return 4;
	}
	case 0x4A:		//LD C, D
	{
		//Put value of register D into register C
		registers.c = registers.d;
		registers.pc++;
		return 4;
	}
	case 0x4B:		//LD C, E
	{
		//Put value of register E into register C
		registers.c = registers.e;
		registers.pc++;
		return 4;
	}
	case 0x4C:		//LD C, H
	{
		//Put value of register H into register C
		registers.c = registers.h;
		registers.pc++;
		return 4;
	}
	case 0x4D:		//LD C, L
	{
		//Put value of register L into register C
		registers.c = registers.l;
		registers.pc++;
		return 4;
	}
	case 0x4E:		//LD C, (HL)
	{
		//put value at adress HL into register C
		registers.c = MEM->read(registers.hl);
		registers.pc++;
		return 8;
	}
	case 0x4F:		//LD C, A
	{
		//Put value of register A into register C
		registers.c = registers.a;
		registers.pc++;
		return 4;
	}
	case 0x50:		//LD D, B
	{
		//Put value of register B into register D
		registers.d = registers.b;
		registers.pc++;
		return 4;
	}
	case 0x51:		//LD D, C
	{
		//Put value of register C into register D
		registers.d = registers.c;
		registers.pc++;
		return 4;
	}
	case 0x52:		//LD D, D
	{
		//Put value of register D into register D
		//registers.d = registers.d;
		registers.pc++;
		return 4;
	}
	case 0x53:		//LD D, E
	{
		//Put value of register E into register D
		registers.d = registers.e;
		registers.pc++;
		return 4;
	}
	case 0x54:		//LD D, H
	{
		//Put value of register H into register D
		registers.d = registers.h;
		registers.pc++;
		return 4;
	}
	case 0x55:		//LD D, L
	{
		//Put value of register L into register D
		registers.d = registers.l;
		registers.pc++;
		return 4;
	}
	case 0x56:		//LD D, (HL)
	{
		//put value at adress HL into register D
		registers.d = MEM->read(registers.hl);
		registers.pc++;
		return 8;
	}
	case 0x57:		//LD D, A
	{
		//Put value of register A into register D
		registers.d = registers.a;
		registers.pc++;
		return 4;
	}
	case 0x58:		//LD E, B
	{
		//Put value of register B into register E
		registers.e = registers.b;
		registers.pc++;
		return 4;
	}
	case 0x59:		//LD E, C
	{
		//Put value of register C into register E
		registers.e = registers.c;
		registers.pc++;
		return 4;
	}
	case 0x5A:		//LD E, D
	{
		//Put value of register D into register E
		registers.e = registers.d;
		registers.pc++;
		return 4;
	}
	case 0x5B:		//LD E, E
	{
		//Put value of register E into register E
		//registers.e = registers.e;
		registers.pc++;
		return 4;
	}
	case 0x5C:		//LD E, H
	{
		//Put value of register H into register E
		registers.e = registers.h;
		registers.pc++;
		return 4;
	}
	case 0x5D:		//LD E, L
	{
		//Put value of register L into register E
		registers.e = registers.l;
		registers.pc++;
		return 4;
	}
	case 0x5E:		//LD E, (HL)
	{
		//put value at adress HL into register E
		registers.e = MEM->read(registers.hl);
		registers.pc++;
		return 8;
	}
	case 0x5F:		//LD E, A
	{
		//Put value of register A into register E
		registers.e = registers.a;
		registers.pc++;
		return 4;
	}
	case 0x60:		//LD H, B
	{
		//Put value of register B into register H
		registers.h = registers.b;
		registers.pc++;
		return 4;
	}
	case 0x61:		//LD H, C
	{
		//Put value of register C into register H
		registers.h = registers.c;
		registers.pc++;
		return 4;
	}
	case 0x62:		//LD H, D
	{
		//Put value of register D into register H
		registers.h = registers.d;
		registers.pc++;
		return 4;
	}
	case 0x63:		//LD H, E
	{
		//Put value of register E into register H
		registers.h = registers.e;
		registers.pc++;
		return 4;
	}
	case 0x64:		//LD H, H
	{
		//Put value of register H into register H
		//registers.h = registers.h;
		registers.pc++;
		return 4;
	}
	case 0x65:		//LD H, L
	{
		//Put value of register L into register H
		registers.h = registers.l;
		registers.pc++;
		return 4;
	}
	case 0x66:		//LD H, (HL)
	{
		//put value at adress HL into register H
		registers.h = MEM->read(registers.hl);
		registers.pc++;
		return 8;
	}
	case 0x67:		//LD H, A
	{
		//Put value of register A into register H
		registers.h = registers.a;
		registers.pc++;
		return 4;
	}
	case 0x68:		//LD L, B
	{
		//Put value of register B into register L
		registers.l = registers.b;
		registers.pc++;
		return 4;
	}
	case 0x69:		//LD L, C
	{
		//Put value of register C into register L
		registers.l = registers.c;
		registers.pc++;
		return 4;
	}
	case 0x6A:		//LD L, D
	{
		//Put value of register D into register L
		registers.l = registers.d;
		registers.pc++;
		return 4;
	}
	case 0x6B:		//LD L, E
	{
		//Put value of register E into register L
		registers.l = registers.e;
		registers.pc++;
		return 4;
	}
	case 0x6C:		//LD L, H
	{
		//Put value of register H into register L
		registers.l = registers.h;
		registers.pc++;
		return 4;
	}
	case 0x6D:		//LD L, L
	{
		//Put value of register L into register L
		//registers.l = registers.l;
		registers.pc++;
		return 4;
	}
	case 0x6E:		//LD L, (HL)
	{
		//put value at adress HL into register L
		registers.l = MEM->read(registers.hl);
		registers.pc++;
		return 8;
	}
	case 0x6F:		//LD L, A
	{
		//Put value of register A into register L
		registers.l = registers.a;
		registers.pc++;
		return 4;
	}
	case 0x70:		//LD (HL), B
	{
		//Put value of register B into MEM at (HL)
		MEM->write(registers.hl, registers.b);
		registers.pc++;
		return 8;
	}
	case 0x71:		//LD (HL), C
	{
		//Put value of register C into MEM at (HL)
		MEM->write(registers.hl, registers.c);
		registers.pc++;
		return 8;
	}
	case 0x72:		//LD (HL), D
	{
		//Put value of register D into MEM at (HL)
		MEM->write(registers.hl, registers.d);
		registers.pc++;
		return 8;
	}
	case 0x73:		//LD (HL), E
	{
		//Put value of register E into MEM at (HL)
		MEM->write(registers.hl, registers.e);
		registers.pc++;
		return 8;
	}
	case 0x74:		//LD (HL), H
	{
		//Put value of register H into MEM at (HL)
		MEM->write(registers.hl, registers.h);
		registers.pc++;
		return 8;
	}
	case 0x75:		//LD (HL), L
	{
		//Put value of register L into MEM at (HL)
		MEM->write(registers.hl, registers.l);
		registers.pc++;
		return 8;
	}
	case 0x76:		//HALT
	{
		//Stop!
		registers.pc++;
		halted = true;
		// Failure(3);
		return 4;
	}
	case 0x77:		//LD (HL), A
	{
		//Put register A into MEM at adress HL
		MEM->write(registers.hl, registers.a);
		registers.pc++;
		return 8;
	}
	case 0x78:		//LD A, B
	{
		//Put value of register B into register A
		registers.a = registers.b;
		registers.pc++;
		return 4;
	}
	case 0x79:		//LD A, C
	{
		//Put value of register C into register A
		registers.a = registers.c;
		registers.pc++;
		return 4;
	}
	case 0x7A:		//LD A, D
	{
		//Put value of register D into register A
		registers.a = registers.d;
		registers.pc++;
		return 4;
	}
	case 0x7B:		//LD A, E
	{
		//Put value of register E into register A
		registers.a = registers.e;
		registers.pc++;
		return 4;
	}
	case 0x7C:		//LD A, H
	{
		//Put value of register H into register A
		registers.a = registers.h;
		registers.pc++;
		return 4;
	}
	case 0x7D:		//LD A, L
	{
		//Put value of register L into register A
		registers.a = registers.l;
		registers.pc++;
		return 4;
	}
	case 0x7E:		//LD A, (HL)
	{
		//Put value at adress (HL) into register A
		registers.a = MEM->read(registers.hl);
		registers.pc++;
		return 8;
	}
	case 0x7F:		//LD A, A
	{
		//Put value of register A into register A
		registers.a = registers.a;
		registers.pc++;
		return 4;
	}
	case 0x80:		//ADD A, B
	{
		//Add B to A
		setN(0);
		setC(((int)registers.b + (int)registers.a) > 255);
		setH(((registers.b & 0x0F) + (registers.a & 0x0F)) > 15);
		registers.a = registers.a + registers.b;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x81:		//ADD A, C
	{
		//Add C to A
		setN(0);
		setC(((int)registers.c + (int)registers.a) > 255);
		setH(((registers.c & 0x0F) + (registers.a & 0x0F)) > 15);
		registers.a = registers.a + registers.c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x82:		//ADD A, D
	{
		//Add D to A
		setN(0);
		setC(((int)registers.d + (int)registers.a) > 255);
		setH(((registers.d & 0x0F) + (registers.a & 0x0F)) > 15);
		registers.a = registers.a + registers.d;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x83:		//ADD A, E
	{
		//Add E to A
		setN(0);
		setC(((int)registers.e + (int)registers.a) > 255);
		setH(((registers.e & 0x0F) + (registers.a & 0x0F)) > 15);
		registers.a = registers.a + registers.e;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x84:		//ADD A, H
	{
		//Add H to A
		setN(0);
		setC(((int)registers.h + (int)registers.a) > 255);
		setH(((registers.h & 0x0F) + (registers.a & 0x0F)) > 15);
		registers.a = registers.a + registers.h;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x85:		//ADD A, L
	{
		//Add L to A
		setN(0);
		setC(((int)registers.l + (int)registers.a) > 255);
		setH(((registers.l & 0x0F) + (registers.a & 0x0F)) > 15);
		registers.a = registers.a + registers.l;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x86:		//ADD A, (HL)
	{
		//Add data in (HL) to A
		uint8_t hlData = MEM->read(registers.hl);
		setN(0);
		setC(((int)hlData + (int)registers.a) > 255);
		setH(((hlData & 0x0F) + (registers.a & 0x0F)) > 15);
		registers.a = registers.a + hlData;
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x87:		//ADD A, A
	{
		//Add A to A
		setN(0);
		setC(registers.a >= 128);
		setH((registers.a & 0x0F) >= 8);
		registers.a = registers.a + registers.a;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x88:		//ADC A, b
	{
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)registers.b) > 255);
		setH(((registers.b & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + registers.b + c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x89:		//ADC A, C
	{
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)registers.c) > 255);
		setH(((registers.c & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + registers.c + c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x8A:		//ADC A, D
	{
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)registers.d) > 255);
		setH(((registers.d & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + registers.d + c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x8B:		//ADC A, E
	{
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)registers.e) > 255);
		setH(((registers.e & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + registers.e + c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x8C:		//ADC A, H
	{
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)registers.h) > 255);
		setH(((registers.h & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + registers.h + c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x8D:		//ADC A, L
	{
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)registers.l) > 255);
		setH(((registers.l & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + registers.l + c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x8E:		//ADC A, (HL)
	{
		uint8_t hlData = MEM->read(registers.hl);
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)hlData) > 255);
		setH(((hlData & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + hlData + c;
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x8F:		//ADC A, A
	{
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)registers.a) > 255);
		setH(((registers.a & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + registers.a + c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x90:      //SUB B
	{
		//Subtract B from A
		setN(1);
		setC(registers.b > registers.a);
		setH((registers.b & 0x0F) > (registers.a & 0x0F));
		registers.a = registers.a - registers.b;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x91:      //SUB C
	{
		//Subtract C from A
		setN(1);
		setC(registers.c > registers.a);
		setH((registers.c & 0x0F) > (registers.a & 0x0F));
		registers.a = registers.a - registers.c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x92:      //SUB D
	{
		//Subtract D from A
		setN(1);
		setC(registers.d > registers.a);
		setH((registers.d & 0x0F) > (registers.a & 0x0F));
		registers.a = registers.a - registers.d;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x93:      //SUB E
	{
		//Subtract E from A
		setN(1);
		setC(registers.e > registers.a);
		setH((registers.e & 0x0F) > (registers.a & 0x0F));
		registers.a = registers.a - registers.e;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x94:      //SUB H
	{
		//Subtract H from A
		setN(1); //set N flag
		setC(registers.h > registers.a);
		setH((registers.h & 0x0F) > (registers.a & 0x0F));
		registers.a = registers.a - registers.h;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x95:		//SUB L
	{
		//Subtract L from A
		setN(1); //set N flag
		setC(registers.l > registers.a);
		setH((registers.l & 0x0F) > (registers.a & 0x0F));
		registers.a = registers.a - registers.l;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x96:		//SUB (HL)
	{
		//Subtract Data at HL from A
		uint8_t hlData = MEM->read(registers.hl);
		setN(1); //set N flag
		setC(hlData > registers.a);
		setH((hlData & 0x0F) > (registers.a & 0x0F));
		registers.a = registers.a - hlData;
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x97:		//SUB A
	{
		//Subtract A from A
		setN(1); //set N flag
		setZ(1);
		setC(0);
		setH(0);
		registers.a = 0x00;
		registers.pc++;
		return 4;
	}
	case 0x98:		//SBC A, B
	{
		//Subtract B + Cflag from A
		bool c = (registers.f & 0b00010000);
		setN(1); //set N flag
		setC(((int)c + (int)registers.b) > (int)registers.a);
		setH(((registers.a & 0x0F) - (registers.b & 0x0F) - c) & 0x10);
		registers.a = registers.a - (registers.b + c);
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x99:		//SBC A, C
	{
		//Subtract C + Cflag from A
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(1); //set N flag
		setC(((int)c + (int)registers.c) > (int)registers.a);
		setH(((registers.a & 0x0F) - (registers.c & 0x0F) - c) & 0x10);
		registers.a = registers.a - registers.c - c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x9A:		//SBC A, D
	{
		//Subtract D + Cflag from A
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(1); //set N flag
		setC(((int)c + (int)registers.d) > (int)registers.a);
		setH(((registers.a & 0x0F) - (registers.d & 0x0F) - c) & 0x10);
		registers.a = registers.a - registers.d - c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x9B:		//SBC A, E
	{
		//Subtract E + Cflag from A
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(1); //set N flag
		setC(((int)c + (int)registers.e) > (int)registers.a);
		setH(((registers.a & 0x0F) - (registers.e & 0x0F) - c) & 0x10);
		registers.a = registers.a - registers.e - c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x9C:		//SBC A, H
	{
		//Subtract H + Cflag from A
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(1); //set N flag
		setC(((int)c + (int)registers.h) > (int)registers.a);
		setH(((registers.a & 0x0F) - (registers.h & 0x0F) - c) & 0x10);
		registers.a = registers.a - registers.h - c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x9D:		//SBC A, L
	{
		//Subtract L + Cflag from A
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(1); //set N flag
		setC(((int)c + (int)registers.l) > (int)registers.a);
		setH(((registers.a & 0x0F) - (registers.l & 0x0F) - c) & 0x10);
		registers.a = registers.a - registers.l - c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0x9E:		//SBC A, (HL)
	{
		//Subtract data at HL + Cflag from A
		uint8_t hlData = MEM->read(registers.hl);
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(1); //set N flag
		setC(((int)c + (int)hlData) > (int)registers.a);
		setH(((registers.a & 0x0F) - (hlData & 0x0F) - c) & 0x10);
		registers.a = registers.a - hlData - c;
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x9F:		//SBC A, A
	{
		//Subtract A + Cflag from A
		bool c = (registers.f & 0b00010000);
		setN(1); //set N flag
		setH(((registers.a & 0x0F) - (registers.a & 0x0F) - c) & 0x10);
		registers.a = !c;
		setZ(!registers.a);
		registers.pc++;
		return 4;
	}
	case 0xA0:		//AND B
	{
		//Logically AND B with A, place result in A
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & registers.b;
		setZ(!registers.a);
		setN(0);
		setH(1);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xA1:		//AND C
	{
		//Logically AND C with A, place result in A
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & registers.c;
		setZ(!registers.a);
		setN(0);
		setH(1);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xA2:		//AND D
	{
		//Logically AND D with A, place result in A
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & registers.d;
		setZ(!registers.a);
		setN(0);
		setH(1);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xA3:		//AND E
	{
		//Logically AND E with A, place result in A
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & registers.e;
		setZ(!registers.a);
		setN(0);
		setH(1);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xA4:		//AND H
	{
		//Logically AND H with A, place result in A
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & registers.h;
		setZ(!registers.a);
		setN(0);
		setH(1);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xA5:		//AND L
	{
		//Logically AND L with A, place result in A
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & registers.l;
		setZ(!registers.a);
		setN(0);
		setH(1);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xA6:		//AND (HL)
	{
		//Logically AND (HL) with A, place result in A
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & MEM->read(registers.hl);
		setZ(!registers.a);
		setN(0);
		setH(1);
		setC(0);
		registers.pc++;
		return 8;
	}
	case 0xA7:		//AND A
	{
		//Logically AND A with A, place result in A
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & registers.a;
		setZ(!registers.a);
		setN(0);
		setH(1);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xA8:		//XOR B
	{
		//logical XOR between A and B.
		registers.a = registers.a ^ registers.b;
		registers.f = 0b00000000; //set zero flag
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xA9:		//XOR C
	{
		//logical XOR between A and C.
		registers.a = registers.a ^ registers.c;
		registers.f = 0b00000000; //set zero flag
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xAA:		//XOR D
	{
		//logical XOR between A and D.
		registers.a = registers.a ^ registers.d;
		registers.f = 0b00000000; //set zero flag
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xAB:		//XOR E
	{
		//logical XOR between A and E.
		registers.a = registers.a ^ registers.e;
		registers.f = 0b00000000; //set zero flag
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xAC:		//XOR H
	{
		//logical XOR between A and H.
		registers.a = registers.a ^ registers.h;
		registers.f = 0b00000000; //set zero flag
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xAD:		//XOR L
	{
		//logical XOR between A and L.
		registers.a = registers.a ^ registers.l;
		registers.f = 0b00000000; //set zero flag
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xAE:		//XOR (HL)
	{
		//logical XOR between A and (HL).
		registers.a = registers.a ^ MEM->read(registers.hl);
		registers.f = 0b00000000; //set zero flag
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 8;
	}
	case 0xAF:      //XOR A
	{
		//logical XOR between A and ... A. so 0 into A
		registers.a = 0x00;
		registers.f = 0b10000000; //set zero flag
		registers.pc++;
		return 4;
	}
	case 0xB0:		//OR B
	{
		//Logical OR Register B with Register A, place result in A
		registers.a = registers.a | registers.b;
		registers.f = 0;
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xB1:		//OR C
	{
		//Logical OR Register C with Register A, place result in A
		registers.a = registers.a | registers.c;
		registers.f = 0;
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xB2:		//OR D
	{
		//Logical OR Register D with Register A, place result in A
		registers.a = registers.a | registers.d;
		registers.f = 0;
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xB3:		//OR E
	{
		//Logical OR Register E with Register A, place result in A
		registers.a = registers.a | registers.e;
		registers.f = 0;
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xB4:		//OR H
	{
		//Logical OR Register H with Register A, place result in A
		registers.a = registers.a | registers.h;
		registers.f = 0;
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xB5:		//OR L
	{
		//Logical OR Register L with Register A, place result in A
		registers.a = registers.a | registers.l;
		registers.f = 0;
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xB6:		//OR (HL)
	{
		//Logical OR data at HL with Register A, place result in A
		registers.a = registers.a | MEM->read(registers.hl);
		registers.f = 0;
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 8;
	}
	case 0xB7:		//OR A
	{
		//Logical OR Register A with Register A, place result in A
		//registers.a = registers.a | registers.a;
		registers.f = 0;
		setZ(!registers.a);
		setN(0);
		setH(0);
		setC(0);
		registers.pc++;
		return 4;
	}
	case 0xB8:		//CP B
	{
		//compare data in B to A
		setN(1); //set N flag
		setC(registers.b > registers.a);
		setH((registers.b & 0x0F) > (registers.a & 0x0F));
		setZ(registers.a == registers.b);
		registers.pc++;
		return 4;
	}
	case 0xB9:		//CP C
	{
		//compare data in C to A
		setN(1); //set N flag
		setC(registers.c > registers.a);
		setH((registers.c & 0x0F) > (registers.a & 0x0F));
		setZ(registers.a == registers.c);
		registers.pc++;
		return 4;
	}
	case 0xBA:		//CP D
	{
		//compare data in D to A
		setN(1); //set N flag
		setC(registers.d > registers.a);
		setH((registers.d & 0x0F) > (registers.a & 0x0F));
		setZ(registers.a == registers.d);
		registers.pc++;
		return 4;
	}
	case 0xBB:		//CP E
	{
		//compare data in E to A
		setN(1); //set N flag
		setC(registers.e > registers.a);
		setH((registers.e & 0x0F) > (registers.a & 0x0F));
		setZ(registers.a == registers.e);
		registers.pc++;
		return 4;
	}
	case 0xBC:		//CP H
	{
		//compare data in H to A
		setN(1); //set N flag
		setC(registers.h > registers.a);
		setH((registers.h & 0x0F) > (registers.a & 0x0F));
		setZ(registers.a == registers.h);
		registers.pc++;
		return 4;
	}
	case 0xBD:		//CP L
	{
		//compare data in L to A
		setN(1); //set N flag
		setC(registers.l > registers.a);
		setH((registers.l & 0x0F) > (registers.a & 0x0F));
		setZ(registers.a == registers.l);
		registers.pc++;
		return 4;
	}
	case 0xBE:		//CP (HL)
	{
		//compare data at (HL) to A
		uint8_t hlData = MEM->read(registers.hl);
		setN(1); //set N flag
		setC(hlData > registers.a);
		setH((hlData & 0x0F) > (registers.a & 0x0F));
		setZ(registers.a == hlData);
		registers.pc++;
		return 8;
	}
	case 0xBF:		//CP A
	{
		//compare data in A to A
		setZ(1); //set Z flag
		setN(1); //set N flag
		setH(0); //set H flag
		setC(0); //set C flag
		registers.pc++;
		return 4;
	}
	case 0xC0:		//RET NZ
	{
		//return if Zflag is reset
		if (!(registers.f & 0b10000000)) {
			int first = PopStack();
			registers.pc = first + ((int)PopStack() << 8);
			return 20;
		}
		else {
			registers.pc++;//jump past parameter
			return 8;
		}
	}
	case 0xC1:		//POP BC
	{
		registers.c = PopStack();
		registers.b = PopStack();
		registers.pc++;
		return 12;
	}
	case 0xC2:		//JP NZ, nn
	{
		//jump to nn if Zflag is reset 
		if (!(registers.f & 0b10000000)) {
			registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
			return 12;
		}
		else {
			registers.pc++;
			registers.pc++;//jump past parameter
			registers.pc++;
			return 8;
		}
	}
	case 0xC3:      //JP nn
	{
		//jump to adress nn (lsByte first)
		registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
		return 12;
	}
	case 0xC4:		//CALL NZ, nn
	{
		//Call to nn if Zflag is reset 
		if (!(registers.f & 0b10000000)) {
			PushStack(((registers.pc + 3) & 0xFF00) >> 8);
			PushStack((registers.pc + 3) & 0x00FF);
			registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
			return 24;
		}
		else {
			registers.pc++;
			registers.pc++;//jump past parameter
			registers.pc++;
			return 12;
		}
	}
	case 0xC5:		//PUSH BC
	{
		registers.pc++;
		PushStack(registers.b);
		PushStack(registers.c);
		return 16;
	}
	case 0xC6:		//ADD A, n
	{
		//Add n to A
		setN(0);
		registers.pc++;
		setC(((int)dMEM[registers.pc] + (int)registers.a) > 255);
		setH(((dMEM[registers.pc] & 0x0F) + (registers.a & 0x0F)) > 15);
		registers.a = registers.a + dMEM[registers.pc];
		setZ(!registers.a);
		registers.pc++; //jump past parameter
		return 8;
	}
	case 0xC7:		//RST 00H
	{
		//Put next address on stack
		//set PC on 0x0000
		registers.pc++;
		PushStack((registers.pc & 0xFF00) >> 8);
		PushStack(registers.pc & 0x00FF);
		//TODO: Test this please
		registers.pc = 0x0000;
		return 32;
	}
	case 0xC8:		//RET Z
	{
		//return if Zflag is set
		if (registers.f & 0b10000000) {
			int first = PopStack();
			registers.pc = first + ((int)PopStack() << 8);
			return 20;
		}
		else {
			registers.pc++; //jump past parameter
			return 8;
		}
	}
	case 0xC9:		//RET
	{
		//return from sub routine, pop two bytes off stack and jump to that adress;
		int first = PopStack();
		registers.pc = (first) + ((int)PopStack() << 8);
		return 16;
	}
	case 0xCA:		//JP Z, nn
	{
		//jump to nn if Zflag is set 
		if (registers.f & 0b10000000) {
			registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
			return 12;
		}
		else {
			registers.pc++;
			registers.pc++;//jump past parameter
			registers.pc++;
			return 8;
		}
	}
	case 0xCB:		//THE GREAT AND TERRIFYING PREFIX
	{
		//run all the extra op codes with this prefix
		registers.pc++;
		return CBPrefix();
	}
	case 0xCC:		//CALL Z, nn
	{
		//Call to nn if Zflag is set 
		if (registers.f & 0b10000000) {
			PushStack(((registers.pc + 3) & 0xFF00) >> 8);
			PushStack((registers.pc + 3) & 0x00FF);
			registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
			return 24;
		}
		else {
			registers.pc++;
			registers.pc++;//jump past parameters
			registers.pc++;
			return 12;
		}
		return 4;
	}
	case 0xCD:		//CALL (nn)
	{
		//push adress of next instruction onto stack and then jump to adress nn
		PushStack(((registers.pc + 3) & 0xFF00) >> 8);
		PushStack((registers.pc + 3) & 0x00FF);
		registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
		return 12;
	}
	case 0xCE:		//ADC A, n
	{
		registers.pc++;
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(0);
		setC(((int)c + (int)registers.a + (int)dMEM[registers.pc]) > 255);
		setH(((dMEM[registers.pc] & 0x0F) + (registers.a & 0x0F) + c) > 15);
		registers.a = registers.a + dMEM[registers.pc] + c;
		setZ(!registers.a);
		registers.pc++; //Increment past param
		return 8;
	}
	case 0xCF:		//RST 08H
	{
		//Put next address on stack
		//set PC on 0x0008
		registers.pc++;
		PushStack((registers.pc & 0xFF00) >> 8);
		PushStack(registers.pc & 0x00FF);
		//TODO: Test this please
		registers.pc = 0x0008;
		return 32;
	}
	case 0xD0:		//RET NC
	{
		//return if Cflag is reset
		if (!(registers.f & 0b00010000)) {
			int first = PopStack();
			registers.pc = first + ((int)PopStack() << 8);
			return 20;
		}
		else {
			registers.pc++;//jump past parameter
			return 8;
		}
	}
	case 0xD1:		//POP DE
	{
		registers.e = PopStack();
		registers.d = PopStack();
		registers.pc++;
		return 12;
	}
	case 0xD2:		//JP NC, nn
	{
		//jump to nn if Cflag is reset 
		if (!(registers.f & 0b00010000)) {
			registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
			return 12;
		}
		else {
			registers.pc++;
			registers.pc++;//jump past parameter
			registers.pc++;
			return 8;
		}
	}
	case 0xD3:		//NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xD4:		//CALL NC, nn
	{
		//Call to nn if Cflag is reset 
		if (!(registers.f & 0b00010000)) {
			PushStack(((registers.pc + 3) & 0xFF00) >> 8);
			PushStack((registers.pc + 3) & 0x00FF);
			registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
			return 24;
		}
		else {
			registers.pc++;
			registers.pc++;//jump past parameter
			registers.pc++;
			return 12;
		}
	}
	case 0xD5:		//PUSH DE
	{
		//push DE onto the stack
		registers.pc++;
		PushStack(registers.d);
		PushStack(registers.e);
		return 16;
	}
	case 0xD6:      //SUB n
	{
		//Subtract n from A
		registers.pc++;
		setN(1);
		setC(dMEM[registers.pc] > registers.a);
		setH((dMEM[registers.pc] & 0x0F) > (registers.a & 0x0F));
		registers.a = registers.a - dMEM[registers.pc];
		setZ(!(registers.a));
		registers.pc++;//Jump past param
		return 8;
	}
	case 0xD7:		//RST 10H;
	{
		//Put next address on stack
		//set PC on 0x0010
		registers.pc++;
		PushStack((registers.pc & 0xFF00) >> 8);
		PushStack(registers.pc & 0x00FF);
		//TODO: Test this please
		registers.pc = 0x0010;
		return 32;
	}
	case 0xD8:		//RET C
	{
		//return if Zflag is set
		if (registers.f & 0b00010000) {
			int first = PopStack();
			registers.pc = first + ((int)PopStack() << 8);
			return 20;
		}
		else {
			registers.pc++;
			return 8;
		}
	}
	case 0xD9:		//RETI
	{
		//return from Interrupt, pop two bytes off stack and jump to that adress. then re-enable interrupts
		int first = PopStack();
		registers.pc = first + ((int)PopStack() << 8);
		preIME = 1;
		//std::cout << "Return from interrupt";
		return 16;
	}
	case 0xDA:		//JP C, nn
	{
		//jump to nn if Cflag is set 
		if (registers.f & 0b00010000) {
			registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
			return 12;
		}
		else {
			registers.pc++;
			registers.pc++;//jump past parameter
			registers.pc++;
			return 8;
		}
	}
	case 0xDB:		//NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xDC:		//CALL C, nn
	{
		//Call to nn if Cflag is set 
		if (registers.f & 0b00010000) {
			PushStack(((registers.pc + 3) & 0xFF00) >> 8);
			PushStack((registers.pc + 3) & 0x00FF);
			registers.pc = (dMEM[registers.pc + 1] + (dMEM[registers.pc + 2] << 8));
			return 24;
		}
		else {
			registers.pc++;
			registers.pc++;//jump past parameter
			registers.pc++;
			return 12;
		}
		return 4;
	}
	case 0xDD:		//NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xDE:		//SBC A, n
	{
		//Subtract n + Cflag from A
		registers.pc++;
		bool c = ((registers.f & 0b00010000) >> 4);
		setN(1); //set N flag
		setC(((int)c + (int)dMEM[registers.pc]) > (int)registers.a);
		setH(((dMEM[registers.pc] & 0x0F) + c) > (registers.a & 0x0F));
		registers.a = registers.a - dMEM[registers.pc] - c;
		setZ(!registers.a);
		registers.pc++; // Increment past param
		return 8;
	}
	case 0xDF:		//RST 18H;
	{
		//Put next address on stack
		//set PC on 0x0018
		registers.pc++;
		PushStack((registers.pc & 0xFF00) >> 8);
		PushStack(registers.pc & 0x00FF);
		//TODO: Test this please
		registers.pc = 0x0018;
		return 32;
	}
	case 0xE0:      //LDH (n), A
	{
		//put A into MEM adress $FF00(IOports) + n
		registers.pc++;
		dMEM[0xFF00 + dMEM[registers.pc]] = registers.a;
		registers.pc++;//increment past param
		return 12;
	}
	case 0xE1:		//POP HL
	{
		registers.l = PopStack();
		registers.h = PopStack();
		registers.pc++;
		return 12;
	}
	case 0xE2:		//LD (C), A
	{
		//put register A into adress $FF00 + register C
		dMEM[0xFF00 + registers.c] = registers.a;
		registers.pc++;
		return 8;
	}
	case 0xE3:      //NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xE4:      //NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xE5:		//PUSH HL
	{
		PushStack(registers.h);
		PushStack(registers.l);
		registers.pc++;
		return 16;
	}
	case 0xE6:		//AND n
	{
		//Logically AND n with A, place result in A
		registers.pc++;
		registers.f = 0b00100000;//reset N, set H, reset C
		registers.a = registers.a & dMEM[registers.pc];
		setZ(!registers.a);
		registers.pc++; // Inc past Parameter n
		return 8;
	}
	case 0xE7:		//RST 20H;
	{
		//Put next address on stack
		//set PC on 0x0020
		registers.pc++;
		PushStack((registers.pc & 0xFF00) >> 8);
		PushStack(registers.pc & 0x00FF);
		//TODO: Test this please
		registers.pc = 0x0020;
		return 32;
	}
	case 0xE8:		//ADD SP, n
	{
		//Add n to SP
		registers.pc++;
		setZ(0);
		setN(0);
		signed char n = ((signed char)dMEM[registers.pc]);
		//setC(((int)registers.sp + n) > 0xFFFF || ((int)registers.sp + n) < 0x0);
		setC(((registers.sp & 0xFF) + (n & 0xFF)) & 0x100);
		setH(((registers.sp & 0x0F) + (n & 0x0F)) & 0x10);
		//setH(((registers.a & 0x0F) - (registers.h & 0xf) - (c & 0x0F)) & 0x10);
		registers.sp = registers.sp + n;
		registers.pc++;
		return 16;
	}
	case 0xE9:		//JP HL
	{
		//jump to the adress in HL
		registers.pc = registers.hl;
		return 4;
	}
	case 0xEA:		//LD (nn), A
	{
		//put A into (nn)
		registers.pc++;
		MEM->write(dMEM[registers.pc] + (dMEM[registers.pc + 1] << 8), registers.a);
		registers.pc++;//increment past params
		registers.pc++;
		return 16;
	}
	case 0xEB:      //NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xEC:		//NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xED:		//NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xEE:		//XOR n
	{
		//logical XOR between A and n.
		registers.pc++;
		registers.a = registers.a ^ dMEM[registers.pc];
		registers.f = 0b00000000; //set zero flag
		setZ(!registers.a);
		registers.pc++;//inc past Param n
		return 8;
	}
	case 0xEF:		//RST 28H;
	{
		//Put next address on stack
		//set PC on 0x0028
		registers.pc++;
		PushStack((registers.pc & 0xFF00) >> 8);
		PushStack(registers.pc & 0x00FF);
		//TODO: Test this please
		registers.pc = 0x0028;
		return 32;
	}
	case 0xF0:      //LDH A, n
	{
		//put MEM adress $FF00(IOports) + n into register A
		registers.pc++;
		registers.a = dMEM[0xFF00 + dMEM[registers.pc]];
		registers.pc++;//increment past param
		return 12;
	}
	case 0xF1:		//POP AF
	{
		uint8_t f = PopStack();
		setZ(f & 0b10000000);
		setN(f & 0b01000000);
		setH(f & 0b00100000);
		setC(f & 0b00010000);
		registers.a = PopStack();
		registers.pc++;
		return 12;
	}
	case 0xF2:		//LD A, (C)
	{
		//put $FF00 + register C into register A
		registers.a = dMEM[0xFF00 + registers.c];
		registers.pc++;
		return 8;
	}
	case 0xF3:      //DI
	{
		//disable interupts after instruction is complete
		preIME = 0;
		registers.pc++;
		return 4;
	}
	case 0xF4:		//NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xF5:		//PUSH AF
	{
		registers.pc++;
		PushStack(registers.a);
		PushStack(registers.f);
		return 16;
	}
	case 0xF6:		//OR n
	{
		//Logically OR n with A, place result in A
		registers.pc++;
		registers.f = 0b00000000;//reset N, set H, reset C
		registers.a = registers.a | dMEM[registers.pc];
		setZ(!registers.a);
		registers.pc++; //jump past parameter
		return 8;
	}
	case 0xF7:      //RST 30H
	{
		//Put current address on stack
		//set PC on 0x0030
		registers.pc++;
		PushStack((registers.pc & 0xFF00) >> 8);
		PushStack(registers.pc & 0x00FF);
		//TODO: Test this please
		registers.pc = 0x0030;
		return 32;
	}
	case 0xF8:		//LD HL, SP + n
	{
		//load SP + n(signed) into HL
		registers.pc++;
		setZ(0);
		setN(0);
		signed char n = ((signed char)dMEM[registers.pc]);
		//setC(((int)registers.sp + n) > 0xFFFF || ((int)registers.sp + n) < 0x0);
		setC(((registers.sp & 0xFF) + (n & 0xFF)) & 0x100);
		setH(((registers.sp & 0x0F) + (n & 0x0F)) & 0x10);
		//setC(((int)registers.sp + ((signed int)MEM[registers.pc])) > 0xFFFF);
		//setH(((int)(registers.sp&0x0FFF) + ((signed int)(MEM[registers.pc]))) > 0x0FFF);
		registers.hl = registers.sp + n;
		registers.pc++;
		return 12;
	}
	case 0xF9:		//LD SP, HL
	{
		registers.sp = registers.hl;
		registers.pc++;
		return 8;
	}
	case 0xFA:		//LD A, (nn)
	{
		//Load value at the adress nn into A;
		registers.pc++;
		registers.a = MEM->read(dMEM[registers.pc] + (dMEM[registers.pc + 2] << 8));
		registers.pc++; //jump past params
		registers.pc++;
		return 16;
	}
	case 0xFB:      //EI
	{
		//enable interupts after next instruction is complete
		preIME = 1;
		registers.pc++;
		return 4;
	}
	case 0xFC:		//NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xFD:		//NO INSTRUCTION
	{
		Failure(1);
		return 0;
	}
	case 0xFE:      //CP n
	{
		//Compare A with n
		//like an A-n instruction but forget the result
		registers.pc++;
		uint8_t n = dMEM[registers.pc];
		setN(1);
		setC(n > registers.a);
		setH((n & 0x0F) > (registers.a & 0x0F));
		setZ(n == registers.a);
		registers.pc++;//jump past parameter
		return 8;
	}
	case 0xFF:      //RST 38H
	{
		//Put current address on stack
		//set PC on 0x0038
		registers.pc++;
		PushStack((registers.pc & 0xFF00) >> 8);
		PushStack(registers.pc & 0x00FF);
		//TODO: Test this please
		registers.pc = 0x0038;
		return 32;
	}
	default:
	{
		Failure(0);
		return 0;
	}
	}
}

uint8_t gbCPU::CBPrefix() {
	switch (dMEM[registers.pc]) {
	case 0x00:		//RLC B
	{
		//Rotate B Left, old bit 7 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.b & 0b10000000);
		setC(n);
		registers.b = (registers.b & 0x7F) << 1;
		registers.b += n;
		setZ(!(registers.b));
		registers.pc++;
		return 8;
	}
	case 0x01:		//RLC C
	{
		//Rotate C Left, old bit 7 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.c & 0b10000000);
		setC(n);
		registers.c = (registers.c & 0x7F) << 1;
		registers.c += n;
		setZ(!(registers.c));
		registers.pc++;
		return 8;
	}
	case 0x02:		//RLC D
	{
		//Rotate D Left, old bit 7 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.d & 0b10000000);
		setC(n);
		registers.d = (registers.d & 0x7F) << 1;
		registers.d += n;
		setZ(!(registers.d));
		registers.pc++;
		return 8;
	}
	case 0x03:		//RLC E
	{
		//Rotate E Left, old bit 7 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.e & 0b10000000);
		setC(n);
		registers.e = (registers.e & 0x7F) << 1;
		registers.e += n;
		setZ(!(registers.e));
		registers.pc++;
		return 8;
	}
	case 0x04:		//RLC H
	{
		//Rotate H Left, old bit 7 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.h & 0b10000000);
		setC(n);
		registers.h = (registers.h & 0x7F) << 1;
		registers.h += n;
		setZ(!(registers.h));
		registers.pc++;
		return 8;
	}
	case 0x05:		//RLC L
	{
		//Rotate L Left, old bit 7 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.l & 0b10000000);
		setC(n);
		registers.l = (registers.l & 0x7F) << 1;
		registers.l += n;
		setZ(!(registers.l));
		registers.pc++;
		return 8;
	}
	case 0x06:		//RLC (HL)
	{
		//Rotate Data at HL Left, old bit 7 into carry flag
		setN(0);
		setH(0);
		uint8_t hlData = MEM->read(registers.hl);
		bool n = (hlData &0b10000000);
		setC(n);
		hlData = (hlData & 0x7F) << 1;
		hlData += n;
		setZ(!(hlData));
		MEM->write(registers.hl, hlData);
		registers.pc++;
		return 8;
	}
	case 0x07:		//RLC A
	{
		//Rotate A Left, old bit 7 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.a & 0b10000000);
		setC(n);
		registers.a = (registers.a & 0x7F) << 1;
		registers.a += n;
		setZ(!(registers.a));
		registers.pc++;
		return 8;
	}
	case 0x08:		//RRC B
	{
		//Rotate B right, old bit 0 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.b & 0b00000001);
		setC(n);
		registers.b = (registers.b & 0xFE) >> 1;
		registers.b += ((int)n) << 7;
		setZ(!(registers.b));
		registers.pc++;
		return 8;
	}
	case 0x09:		//RRC C
	{
		//Rotate C right, old bit 0 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.c & 0b00000001);
		setC(n);
		registers.c = (registers.c & 0xFE) >> 1;
		registers.c += ((int)n) << 7;
		setZ(!(registers.c));
		registers.pc++;
		return 8;
	}
	case 0x0A:		//RRC D
	{
		//Rotate D right, old bit 0 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.d & 0b00000001);
		setC(n);
		registers.d = (registers.d & 0xFE) >> 1;
		registers.d += ((int)n) << 7;
		setZ(!(registers.d));
		registers.pc++;
		return 8;
	}
	case 0x0B:		//RRC E
	{
		//Rotate E right, old bit 0 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.e & 0b00000001);
		setC(n);
		registers.e = (registers.e & 0xFE) >> 1;
		registers.e += ((int)n) << 7;
		setZ(!(registers.e));
		registers.pc++;
		return 8;
	}
	case 0x0C:		//RRC H
	{
		//Rotate H right, old bit 0 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.h & 0b00000001);
		setC(n);
		registers.h = (registers.h & 0xFE) >> 1;
		registers.h += ((int)n) << 7;
		setZ(!(registers.h));
		registers.pc++;
		return 8;
	}
	case 0x0D:		//RRC L
	{
		//Rotate L right, old bit 0 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.l & 0b00000001);
		setC(n);
		registers.l = (registers.l & 0xFE) >> 1;
		registers.l += ((int)n) << 7;
		setZ(!(registers.l));
		registers.pc++;
		return 8;
	}
	case 0x0E:		//RRC (HL)
	{
		//Rotate data in HL right, old bit 0 into carry flag
		setN(0);
		setH(0);
		uint8_t hlData = MEM->read(registers.hl);
		bool n = (hlData & 0b00000001);
		setC(n);
		hlData = (hlData & 0xFE) >> 1;
		hlData += ((int)n) << 7;
		setZ(!(hlData));
		MEM->write(registers.hl, hlData);
		registers.pc++;
		return 8;
	}
	case 0x0F:		//RRC A
	{
		//Rotate A right, old bit 0 into carry flag
		setN(0);
		setH(0);
		bool n = (registers.a & 0b00000001);
		setC(n);
		registers.a = (registers.a & 0xFE) >> 1;
		registers.a += ((int)n) << 7;
		setZ(!(registers.a));
		registers.pc++;
		return 8;
	}
	case 0x10:		//RL B
	{
		//Rotate C left through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.b & 0x80);
		registers.b = (registers.b & 0x7F) << 1;
		registers.b += n;
		setZ(!registers.b);
		registers.pc++;
		return 8;
	}
	case 0x11:		//RL C
	{
		//Rotate C left through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.c & 0x80);
		registers.c = (registers.c & 0x7F) << 1;
		registers.c += n;
		setZ(!registers.c);
		registers.pc++;
		return 8;
	}
	case 0x12:		//RL D
	{
		//Rotate D left through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.d & 0x80);
		registers.d = (registers.d & 0x7F) << 1;
		registers.d += n;
		setZ(!registers.d);
		registers.pc++;
		return 8;
	}
	case 0x13:		//RL E
	{
		//Rotate E left through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.e & 0x80);
		registers.e = (registers.e & 0x7F) << 1;
		registers.e += n;
		setZ(!registers.e);
		registers.pc++;
		return 8;
	}
	case 0x14:		//RL H
	{
		//Rotate H left through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.h & 0x80);
		registers.h = (registers.h & 0x7F) << 1;
		registers.h += n;
		setZ(!registers.h);
		registers.pc++;
		return 8;
	}
	case 0x15:		//RL L
	{
		//Rotate L left through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.l & 0x80);
		registers.l = (registers.l & 0x7F) << 1;
		registers.l += n;
		setZ(!registers.l);
		registers.pc++;
		return 8;
	}
	case 0x16:		//RL (HL)
	{
		//Rotate Data at HL left through Carry Flag
		setN(0);
		setH(0);
		uint8_t hlData = MEM->read(registers.hl);
		bool n = (registers.f & 0b00010000);
		setC(hlData & 0x80);
		hlData = (hlData & 0x7F) << 1;
		hlData += n;
		setZ(!hlData);
		MEM->write(registers.hl, hlData);
		registers.pc++;
		return 8;
	}
	case 0x17:		//RL A
	{
		//Rotate A Left through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.a & 0x80);
		registers.a = (registers.a & 0x7F) << 1;
		registers.a += n;
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x18:		//RR B
	{
		//Rotate B right through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.b & 0x01);
		registers.b = ((registers.b & 0xFE) >> 1);
		registers.b += (((int)n) << 7);
		setZ(!registers.b);
		registers.pc++;
		return 8;
	}
	case 0x19:		//RR C
	{
		//Rotate C right through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.c & 0x01);
		registers.c = ((registers.c & 0xFE) >> 1);
		registers.c += (((int)n)<<7);
		setZ(!registers.c);
		registers.pc++;
		return 8;
	}
	case 0x1A:		//RR D
	{
		//Rotate D right through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.d & 0x01);
		registers.d = ((registers.d & 0xFE) >> 1);
		registers.d += (((int)n) << 7);
		setZ(!registers.d);
		registers.pc++;
		return 8;
	}
	case 0x1B:		//RR E
	{
		//Rotate E right through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.e & 0x01);
		registers.e = ((registers.e & 0xFE) >> 1);
		registers.e += (((int)n) << 7);
		setZ(!registers.e);
		registers.pc++;
		return 8;
	}
	case 0x1C:		//RR H
	{
		//Rotate H right through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.h & 0x01);
		registers.h = ((registers.h & 0xFE) >> 1);
		registers.h += (((int)n) << 7);
		setZ(!registers.h);
		registers.pc++;
		return 8;
	}
	case 0x1D:		//RR L
	{
		//Rotate L right through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.l & 0x01);
		registers.l = ((registers.l & 0xFE) >> 1);
		registers.l += (((int)n) << 7);
		setZ(!registers.l);
		registers.pc++;
		return 8;
	}
	case 0x1E:		//RR (HL)
	{
		//Rotate data at HL right through Carry Flag
		setN(0);
		setH(0);
		uint8_t hlData = MEM->read(registers.hl);
		bool n = (registers.f & 0b00010000);
		setC(hlData & 0x01);
		hlData = ((hlData & 0xFE) >> 1);
		hlData += (((int)n) << 7);
		setZ(!hlData);
		registers.pc++;
		return 8;
	}
	case 0x1F:		//RR A
	{
		//Rotate A right through Carry Flag
		setN(0);
		setH(0);
		bool n = (registers.f & 0b00010000);
		setC(registers.a & 0x01);
		registers.a = ((registers.a & 0xFE) >> 1);
		registers.a += (((int)n) << 7);
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x20:		//SLA B
	{
		//Shift B Left, into carry, LSB is 0;
		setN(0);
		setH(0);
		setC(registers.b & 0x80);
		registers.b = (registers.b & 0x7F) << 1;
		setZ(!registers.b);
		registers.pc++;
		return 8;
	}
	case 0x21:		//SLA C
	{
		//Shift C Left, into carry, LSB is 0;
		setN(0);
		setH(0);
		setC(registers.c & 0x80);
		registers.c = (registers.c & 0x7F) << 1;
		setZ(!registers.c);
		registers.pc++;
		return 8;
	}
	case 0x22:		//SLA D
	{
		//Shift D Left, into carry, LSB is 0;
		setN(0);
		setH(0);
		setC(registers.d & 0x80);
		registers.d = (registers.d & 0x7F) << 1;
		setZ(!registers.d);
		registers.pc++;
		return 8;
	}
	case 0x23:		//SLA E
	{
		//Shift E Left, into carry, LSB is 0;
		setN(0);
		setH(0);
		setC(registers.e & 0x80);
		registers.e = (registers.e & 0x7F) << 1;
		setZ(!registers.e);
		registers.pc++;
		return 8;
	}
	case 0x24:		//SLA H
	{
		//Shift H Left, into carry, LSB is 0;
		setN(0);
		setH(0);
		setC(registers.h & 0x80);
		registers.h = (registers.h & 0x7F) << 1;
		setZ(!registers.h);
		registers.pc++;
		return 8;
	}
	case 0x25:		//SLA L
	{
		//Shift L Left, into carry, LSB is 0;
		setN(0);
		setH(0);
		setC(registers.l & 0x80);
		registers.l = (registers.l & 0x7F) << 1;
		setZ(!registers.l);
		registers.pc++;
		return 8;
	}
	case 0x26:		//SLA (HL)
	{
		//Shift data at HL Left, into carry, LSB is 0;
		setN(0);
		setH(0);
		uint8_t hlData = MEM->read(registers.hl);
		setC(hlData & 0x80);
		MEM->write(registers.hl, (hlData & 0x7F) << 1);
		setZ(!hlData);
		registers.pc++;
		return 8;
	}
	case 0x27:		//SLA A
	{
		//Shift A Left, into carry, LSB is 0;
		setN(0);
		setH(0);
		setC(registers.a & 0x80);
		registers.a = (registers.a & 0x7F) << 1;
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x28:		//SRA B
	{
		//Shift B Right, into carry, MSB is Unchanged;
		setN(0);
		setH(0);
		setC(registers.b & 0x01);
		registers.b = ((registers.b & 0xFE) >> 1) + (registers.b & 0x80);
		setZ(!registers.b);
		registers.pc++;
		return 8;
	}
	case 0x29:		//SRA C
	{
		//Shift C Right, into carry, MSB is Unchanged;
		setN(0);
		setH(0);
		setC(registers.c & 0x01);
		registers.c = ((registers.c & 0xFE) >> 1) + (registers.c & 0x80);
		setZ(!registers.c);
		registers.pc++;
		return 8;
	}
	case 0x2A:		//SRA D
	{
		//Shift D Right, into carry, MSB is Unchanged;
		setN(0);
		setH(0);
		setC(registers.d & 0x01);
		registers.d = ((registers.d & 0xFE) >> 1) + (registers.d & 0x80);
		setZ(!registers.d);
		registers.pc++;
		return 8;
	}
	case 0x2B:		//SRA E
	{
		//Shift E Right, into carry, MSB is Unchanged
		setN(0);
		setH(0);
		setC(registers.e & 0x01);
		registers.e = ((registers.e & 0xFE) >> 1) + (registers.e & 0x80);
		setZ(!registers.e);
		registers.pc++;
		return 8;
	}
	case 0x2C:		//SRA H
	{
		//Shift h Right, into carry, MSB is Unchanged
		setN(0);
		setH(0);
		setC(registers.h & 0x01);
		registers.h = ((registers.h & 0xFE) >> 1) + (registers.h & 0x80);
		setZ(!registers.h);
		registers.pc++;
		return 8;
	}
	case 0x2D:		//SRA L
	{
		//Shift L Right, into carry, MSB is Unchanged
		setN(0);
		setH(0);
		setC(registers.l & 0x01);
		registers.l = ((registers.l & 0xFE) >> 1) + (registers.l & 0x80);
		setZ(!registers.l);
		registers.pc++;
		return 8;
	}
	case 0x2E:		//SRA (HL)
	{
		//Shift data at HL Right, into carry, MSB is Unchanged
		setN(0);
		setH(0);
		uint8_t hlData = MEM->read(registers.hl);
		setC(hlData & 0x01);
		MEM->write(registers.hl, ((hlData & 0xFE) >> 1) + (hlData & 0x80));
		setZ(!hlData);
		registers.pc++;
		return 8;
	}
	case 0x2F:		//SRA A
	{
		//Shift A Right, into carry, MSB is Unchanged
		setN(0);
		setH(0);
		setC(registers.a & 0x01);
		registers.a = ((registers.a & 0xFE) >> 1) + (registers.a & 0x80);
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x30:		//SWAP B
	{
		//Swap Upper and Lower Nibbles of B
		registers.f = 0b00000000;
		registers.b = (registers.b << 4) + (registers.b >> 4);
		setZ(!registers.b);
		registers.pc++;
		return 8;
	}
	case 0x31:		//SWAP C
	{
		//Swap Upper and Lower Nibbles of C
		registers.f = 0b00000000;
		registers.c = (registers.c << 4) + (registers.c >> 4);
		setZ(!registers.c);
		registers.pc++;
		return 8;
	}
	case 0x32:		//SWAP D
	{
		//Swap Upper and Lower Nibbles of D
		registers.f = 0b00000000;
		registers.d = (registers.d << 4) + (registers.d >> 4);
		setZ(!registers.d);
		registers.pc++;
		return 8;
	}
	case 0x33:		//SWAP E
	{
		//Swap Upper and Lower Nibbles of E
		registers.f = 0b00000000;
		registers.e = (registers.e << 4) + (registers.e >> 4);
		setZ(!registers.e);
		registers.pc++;
		return 8;
	}
	case 0x34:		//SWAP H
	{
		//Swap Upper and Lower Nibbles of H
		registers.f = 0b00000000;
		registers.h = (registers.h << 4) + (registers.h >> 4);
		setZ(!registers.h);
		registers.pc++;
		return 8;
	}
	case 0x35:		//SWAP L
	{
		//Swap Upper and Lower Nibbles of L
		registers.f = 0b00000000;
		registers.l = (registers.l << 4) + (registers.l >> 4);
		setZ(!registers.l);
		registers.pc++;
		return 8;
	}
	case 0x36:		//SWAP (HL)
	{
		//Swap Upper and Lower Nibbles of A
		uint8_t hlData = MEM->read(registers.hl);
		registers.f = 0b00000000;
		MEM->write(registers.hl, (hlData << 4) + (hlData >> 4));
		setZ(!hlData);
		registers.pc++;
		return 16;
	}
	case 0x37:		//SWAP A
	{
		//Swap Upper and Lower Nibbles of A
		registers.f = 0b00000000;
		registers.a = (registers.a << 4) + (registers.a >> 4);
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x38:		//SRL B
	{
		//Shift B Right, into carry, MSB is 0;
		setN(0);
		setH(0);
		setC(registers.b & 0x01);
		registers.b = (registers.b & 0xFE) >> 1;
		setZ(!registers.b);
		registers.pc++;
		return 8;
	}
	case 0x39:		//SRL C
	{
		//Shift C Right, into carry, MSB is 0;
		setN(0);
		setH(0);
		setC(registers.c & 0x01);
		registers.c = (registers.c & 0xFE) >> 1;
		setZ(!registers.c);
		registers.pc++;
		return 8;
	}
	case 0x3A:		//SRL D
	{
		//Shift D Right, into carry, MSB is 0;
		setN(0);
		setH(0);
		setC(registers.d & 0x01);
		registers.d = (registers.d & 0xFE) >> 1;
		setZ(!registers.d);
		registers.pc++;
		return 8;
	}
	case 0x3B:		//SRL E
	{
		//Shift E Right, into carry, MSB is 0;
		setN(0);
		setH(0);
		setC(registers.e & 0x01);
		registers.e = (registers.e & 0xFE) >> 1;
		setZ(!registers.e);
		registers.pc++;
		return 8;
	}
	case 0x3C:		//SRL H
	{
		//Shift h Right, into carry, MSB is 0;
		setN(0);
		setH(0);
		setC(registers.h & 0x01);
		registers.h = (registers.h & 0xFE) >> 1;
		setZ(!registers.h);
		registers.pc++;
		return 8;
	}
	case 0x3D:		//SRL L
	{
		//Shift L Right, into carry, MSB is 0;
		setN(0);
		setH(0);
		setC(registers.l & 0x01);
		registers.l = (registers.l & 0xFE) >> 1;
		setZ(!registers.l);
		registers.pc++;
		return 8;
	}
	case 0x3E:		//SRL (HL)
	{
		//Shift data at HL Right, into carry, MSB is 0;
		setN(0);
		setH(0);
		uint8_t hlData = MEM->read(registers.hl);
		setC(hlData & 0x01);
		MEM->write(registers.hl, (hlData & 0xFE) >> 1);
		setZ(!hlData);
		registers.pc++;
		return 8;
	}
	case 0x3F:		//SRL A
	{
		//Shift A Right, into carry, MSB is 0;
		setN(0);
		setH(0);
		setC(registers.a & 0x01);
		registers.a = (registers.a & 0xFE) >> 1;
		setZ(!registers.a);
		registers.pc++;
		return 8;
	}
	case 0x40:		//BIT 0, B
	{
		//test bit 0 in B register
		setZ(!(registers.b & 0b00000001));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x41:		//BIT 0, C
	{
		//test bit 0 in C register
		setZ(!(registers.c & 0b00000001));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x42:		//BIT 0, D
	{
		//test bit 0 in D register
		setZ(!(registers.d & 0b00000001));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x43:		//BIT 0, E
	{
		//test bit 0 in E register
		setZ(!(registers.e & 0b00000001));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x44:		//BIT 0, H
	{
		//test bit 0 in H register
		setZ(!(registers.h & 0b00000001));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x45:		//BIT 0, L
	{
		//test bit 0 in L register
		setZ(!(registers.l & 0b00000001));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x46:		//BIT 0, (HL)
	{
		//test bit 0 in data at adress HL
		setZ(!(MEM->read(registers.hl) & 0b00000001));
		setN(0);
		setH(1);
		registers.pc++;
		return 12;
	}
	case 0x47:		//BIT 0, A
	{
		//test bit 0 in A register
		setZ(!(registers.a & 0b00000001));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x48:		//BIT 1, B
	{
		//test bit 1 in B register
		setZ(!(registers.b & 0b00000010));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x49:		//BIT 1, C
	{
		//test bit 1 in C register
		setZ(!(registers.c & 0b00000010));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x4A:		//BIT 1, D
	{
		//test bit 1 in D register
		setZ(!(registers.d & 0b00000010));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x4B:		//BIT 1, E
	{
		//test bit 1 in E register
		setZ(!(registers.e & 0b00000010));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x4C:		//BIT 1, H
	{
		//test bit 1 in H register
		setZ(!(registers.h & 0b00000010));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x4D:		//BIT 1, L
	{
		//test bit 1 in L register
		setZ(!(registers.l & 0b00000010));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x4E:		//BIT 1, (HL)
	{
		//test bit 1 in data at adress HL
		setZ(!(MEM->read(registers.hl) & 0b00000010));
		setN(0);
		setH(1);
		registers.pc++;
		return 12;
	}
	case 0x4F:		//BIT 1, A
	{
		//test bit 1 in A register
		setZ(!(registers.a & 0b00000010));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x50:		//BIT 2, B
	{
		//test bit 2 in B register
		setZ(!(registers.b & 0b00000100));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x51:		//BIT 2, C
	{
		//test bit 2 in C register
		setZ(!(registers.c & 0b00000100));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x52:		//BIT 2, D
	{
		//test bit 2 in D register
		setZ(!(registers.d & 0b00000100));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x53:		//BIT 2, E
	{
		//test bit 2 in E register
		setZ(!(registers.e & 0b00000100));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x54:		//BIT 2, H
	{
		//test bit 2 in H register
		setZ(!(registers.h & 0b00000100));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x55:		//BIT 2, L
	{
		//test bit 2 in L register
		setZ(!(registers.l & 0b00000100));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x56:		//BIT 2, (HL)
	{
		//test bit 2 in data at adress HL
		setZ(!(MEM->read(registers.hl) & 0b00000100));
		setN(0);
		setH(1);
		registers.pc++;
		return 12;
	}
	case 0x57:		//BIT 2, A
	{
		//test bit 2 in A register
		setZ(!(registers.a & 0b00000100));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x58:		//BIT 3, B
	{
		//test bit 3 in B register
		setZ(!(registers.b & 0b00001000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x59:		//BIT 3, C
	{
		//test bit 3 in C register
		setZ(!(registers.c & 0b00001000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x5A:		//BIT 3, D
	{
		//test bit 3 in D register
		setZ(!(registers.d & 0b00001000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x5B:		//BIT 3, E
	{
		//test bit 3 in E register
		setZ(!(registers.e & 0b00001000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x5C:		//BIT 3, H
	{
		//test bit 3 in H register
		setZ(!(registers.h & 0b00001000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x5D:		//BIT 3, L
	{
		//test bit 3 in L register
		setZ(!(registers.l & 0b00001000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x5E:		//BIT 3, (HL)
	{
		//test bit 3 in data at adress HL
		setZ(!(MEM->read(registers.hl) & 0b00001000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x5F:		//BIT 3, A
	{
		//test bit 3 in A register
		setZ(!(registers.a & 0b00001000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x60:		//BIT 4, B
	{
		//test bit 4 in B register
		setZ(!(registers.b & 0b00010000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x61:		//BIT 4, C
	{
		//test bit 4 in C register
		setZ(!(registers.c & 0b00010000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x62:		//BIT 4, D
	{
		//test bit 4 in D register
		setZ(!(registers.d & 0b00010000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x63:		//BIT 4, E
	{
		//test bit 4 in E register
		setZ(!(registers.e & 0b00010000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x64:		//BIT 4, H
	{
		//test bit 4 in H register
		setZ(!(registers.h & 0b00010000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x65:		//BIT 4, L
	{
		//test bit 4 in L register
		setZ(!(registers.l & 0b00010000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x66:		//BIT 4, (HL)
	{
		//test bit 4 in data at adress HL
		setZ(!(MEM->read(registers.hl) & 0b00010000));
		setN(0);
		setH(1);
		registers.pc++;
		return 12;
	}
	case 0x67:		//BIT 4, A
	{
		//test bit 4 in A register
		setZ(!(registers.a & 0b00010000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x68:		//BIT 5, B
	{
		//test bit 5 in B register
		setZ(!(registers.b & 0b00100000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x69:		//BIT 5, C
	{
		//test bit 5 in C register
		setZ(!(registers.c & 0b00100000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x6A:		//BIT 5, D
	{
		//test bit 5 in D register
		setZ(!(registers.d & 0b00100000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x6B:		//BIT 5, E
	{
		//test bit 5 in E register
		setZ(!(registers.e & 0b00100000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x6C:		//BIT 5, H
	{
		//test bit 5 in H register
		setZ(!(registers.h & 0b00100000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x6D:		//BIT 5, L
	{
		//test bit 5 in L register
		setZ(!(registers.l & 0b00100000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x6E:		//BIT 5, (HL)
	{
		//test bit 5 in data at adress HL
		setZ(!(MEM->read(registers.hl) & 0b00100000));
		setN(0);
		setH(1);
		registers.pc++;
		return 12;
	}
	case 0x6F:		//BIT 5, A
	{
		//test bit 5 in A register
		setZ(!(registers.a & 0b00100000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x70:		//BIT 6, B
	{
		//test bit 6 in B register
		setZ(!(registers.b & 0b01000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x71:		//BIT 6, C
	{
		//test bit 6 in C register
		setZ(!(registers.c & 0b01000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x72:		//BIT 6, D
	{
		//test bit 6 in D register
		setZ(!(registers.d & 0b01000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x73:		//BIT 6, E
	{
		//test bit 6 in E register
		setZ(!(registers.e & 0b01000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x74:		//BIT 6, H
	{
		//test bit 6 in H register
		setZ(!(registers.h & 0b01000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x75:		//BIT 6, L
	{
		//test bit 6 in L register
		setZ(!(registers.l & 0b01000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x76:		//BIT 6, (HL)
	{
		//test bit 6 in data at adress HL
		setZ(!(MEM->read(registers.hl) & 0b01000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 12;
	}
	case 0x77:		//BIT 6, A
	{
		//test bit 6 in A register
		setZ(!(registers.a & 0b01000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x78:		//BIT 7, B
	{
		//test bit 7 in B register
		setZ(!(registers.b & 0b10000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x79:		//BIT 7, C
	{
		//test bit 7 in C register
		setZ(!(registers.c & 0b10000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x7A:		//BIT 7, D
	{
		//test bit 7 in D register
		setZ(!(registers.d & 0b10000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x7B:		//BIT 7, E
	{
		//test bit 7 in E register
		setZ(!(registers.e & 0b10000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x7C:		//BIT 7, H
	{
		//test bit 7 in H register
		setZ(!(registers.h & 0b10000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x7D:		//BIT 7, L
	{
		//test bit 7 in L register
		setZ(!(registers.l & 0b10000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x7E:		//BIT 7, (HL)
	{
		//test bit 7 in data at adress HL
		setZ(!(MEM->read(registers.hl) & 0b10000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 12;
	}
	case 0x7F:		//BIT 7, A
	{
		//test bit 7 in A register
		setZ(!(registers.a & 0b10000000));
		setN(0);
		setH(1);
		registers.pc++;
		return 8;
	}
	case 0x80:		//RES 0, B
	{
		//reset bit 0 in register B
		registers.b &= 0b11111110;
		registers.pc++;
		return 8;
	}
	case 0x81:		//RES 0, C
	{
		//reset bit 0 in register C
		registers.c &= 0b11111110;
		registers.pc++;
		return 8;
	}
	case 0x82:		//RES 0, D
	{
		//reset bit 0 in register D
		registers.d &= 0b11111110;
		registers.pc++;
		return 8;
	}
	case 0x83:		//RES 0, E
	{
		//reset bit 0 in register E
		registers.e &= 0b11111110;
		registers.pc++;
		return 8;
	}
	case 0x84:		//RES 0, H
	{
		//reset bit 0 in register H
		registers.h &= 0b11111110;
		registers.pc++;
		return 8;
	}
	case 0x85:		//RES 0, L
	{
		//reset bit 0 in register L
		registers.l &= 0b11111110;
		registers.pc++;
		return 8;
	}
	case 0x86:		//RES 0, (HL)
	{
		//reset bit 0 in register (HL)
		MEM->andWrite(registers.hl, 0b11111110);
		registers.pc++;
		return 16;
	}
	case 0x87:		//RES 0, A
	{
		//reset bit 0 in register A
		registers.a &= 0b11111110;
		registers.pc++;
		return 8;
	}
	case 0x88:		//RES 1, B
	{
		//reset bit 1 in register B
		registers.b &= 0b11111101;
		registers.pc++;
		return 8;
	}
	case 0x89:		//RES 1, C
	{
		//reset bit 1 in register C
		registers.c &= 0b11111101;
		registers.pc++;
		return 8;
	}
	case 0x8A:		//RES 1, D
	{
		//reset bit 1 in register D
		registers.d &= 0b11111101;
		registers.pc++;
		return 8;
	}
	case 0x8B:		//RES 1, E
	{
		//reset bit 1 in register E
		registers.e &= 0b11111101;
		registers.pc++;
		return 8;
	}
	case 0x8C:		//RES 1, H
	{
		//reset bit 1 in register H
		registers.h &= 0b11111101;
		registers.pc++;
		return 8;
	}
	case 0x8D:		//RES 1, L
	{
		//reset bit 1 in register L
		registers.l &= 0b11111101;
		registers.pc++;
		return 8;
	}
	case 0x8E:		//RES 1, (HL)
	{
		//reset bit 1 in register (HL)
		MEM->andWrite(registers.hl, 0b11111101);
		registers.pc++;
		return 16;
	}
	case 0x8F:		//RES 1, A
	{
		//reset bit 1 in register A
		registers.a &= 0b11111101;
		registers.pc++;
		return 8;
	}
	case 0x90:		//RES 2, B
	{
		//reset bit 2 in register B
		registers.b &= 0b11111011;
		registers.pc++;
		return 8;
	}
	case 0x91:		//RES 2, C
	{
		//reset bit 2 in register C
		registers.c &= 0b11111011;
		registers.pc++;
		return 8;
	}
	case 0x92:		//RES 2, D
	{
		//reset bit 2 in register D
		registers.d &= 0b11111011;
		registers.pc++;
		return 8;
	}
	case 0x93:		//RES 2, E
	{
		//reset bit 2 in register E
		registers.e &= 0b11111011;
		registers.pc++;
		return 8;
	}
	case 0x94:		//RES 2, H
	{
		//reset bit 2 in register H
		registers.h &= 0b11111011;
		registers.pc++;
		return 8;
	}
	case 0x95:		//RES 2, L
	{
		//reset bit 2 in register L
		registers.l &= 0b11111011;
		registers.pc++;
		return 8;
	}
	case 0x96:		//RES 2, (HL)
	{
		//reset bit 2 in register (HL)
		MEM->andWrite(registers.hl, 0b11111011);
		registers.pc++;
		return 16;
	}
	case 0x97:		//RES 2, A
	{
		//reset bit 2 in register A
		registers.a &= 0b11111011;
		registers.pc++;
		return 8;
	}
	case 0x98:		//RES 3, B
	{
		//reset bit 3 in register B
		registers.b &= 0b11110111;
		registers.pc++;
		return 8;
	}
	case 0x99:		//RES 3, C
	{
		//reset bit 3 in register C
		registers.c &= 0b11110111;
		registers.pc++;
		return 8;
	}
	case 0x9A:		//RES 3, D
	{
		//reset bit 3 in register D
		registers.d &= 0b11110111;
		registers.pc++;
		return 8;
	}
	case 0x9B:		//RES 3, E
	{
		//reset bit 3 in register E
		registers.e &= 0b11110111;
		registers.pc++;
		return 8;
	}
	case 0x9C:		//RES 3, H
	{
		//reset bit 3 in register H
		registers.h &= 0b11110111;
		registers.pc++;
		return 8;
	}
	case 0x9D:		//RES 3, L
	{
		//reset bit 3 in register L
		registers.l &= 0b11110111;
		registers.pc++;
		return 8;
	}
	case 0x9E:		//RES 3, (HL)
	{
		//reset bit 3 in register (HL)
		MEM->andWrite(registers.hl, 0b11110111);
		registers.pc++;
		return 16;
	}
	case 0x9F:		//RES 3, A
	{
		//reset bit 3 in register A
		registers.a &= 0b11110111;
		registers.pc++;
		return 8;
	}
	case 0xA0:		//RES 4, B
	{
		//reset bit 4 in register B
		registers.b &= 0b11101111;
		registers.pc++;
		return 8;
	}
	case 0xA1:		//RES 4, C
	{
		//reset bit 4 in register C
		registers.c &= 0b11101111;
		registers.pc++;
		return 8;
	}
	case 0xA2:		//RES 4, D
	{
		//reset bit 4 in register D
		registers.d &= 0b11101111;
		registers.pc++;
		return 8;
	}
	case 0xA3:		//RES 4, E
	{
		//reset bit 4 in register E
		registers.e &= 0b11101111;
		registers.pc++;
		return 8;
	}
	case 0xA4:		//RES 4, H
	{
		//reset bit 4 in register H
		registers.h &= 0b11101111;
		registers.pc++;
		return 8;
	}
	case 0xA5:		//RES 4, L
	{
		//reset bit 4 in register L
		registers.l &= 0b11101111;
		registers.pc++;
		return 8;
	}
	case 0xA6:		//RES 4, (HL)
	{
		//reset bit 4 in register (HL)
		MEM->andWrite(registers.hl, 0b11101111);
		registers.pc++;
		return 16;
	}
	case 0xA7:		//RES 4, A
	{
		//reset bit 4 in register A
		registers.a &= 0b11101111;
		registers.pc++;
		return 8;
	}
	case 0xA8:		//RES 5, B
	{
		//reset bit 5 in register B
		registers.b &= 0b11011111;
		registers.pc++;
		return 8;
	}
	case 0xA9:		//RES 5, C
	{
		//reset bit 5 in register C
		registers.c &= 0b11011111;
		registers.pc++;
		return 8;
	}
	case 0xAA:		//RES 5, D
	{
		//reset bit 5 in register D
		registers.d &= 0b11011111;
		registers.pc++;
		return 8;
	}
	case 0xAB:		//RES 5, E
	{
		//reset bit 5 in register E
		registers.e &= 0b11011111;
		registers.pc++;
		return 8;
	}
	case 0xAC:		//RES 5, H
	{
		//reset bit 5 in register H
		registers.h &= 0b11011111;
		registers.pc++;
		return 8;
	}
	case 0xAD:		//RES 5, L
	{
		//reset bit 5 in register L
		registers.l &= 0b11011111;
		registers.pc++;
		return 8;
	}
	case 0xAE:		//RES 5, (HL)
	{
		//reset bit 5 in register (HL)
		MEM->andWrite(registers.hl, 0b11011111);
		registers.pc++;
		return 16;
	}
	case 0xAF:		//RES 5, A
	{
		//reset bit 5 in register A
		registers.a &= 0b11011111;
		registers.pc++;
		return 8;
	}
	case 0xB0:		//RES 6, B
	{
		//reset bit 6 in register B
		registers.b &= 0b10111111;
		registers.pc++;
		return 8;
	}
	case 0xB1:		//RES 6, C
	{
		//reset bit 6 in register C
		registers.c &= 0b10111111;
		registers.pc++;
		return 8;
	}
	case 0xB2:		//RES 6, D
	{
		//reset bit 6 in register D
		registers.d &= 0b10111111;
		registers.pc++;
		return 8;
	}
	case 0xB3:		//RES 6, E
	{
		//reset bit 6 in register E
		registers.e &= 0b10111111;
		registers.pc++;
		return 8;
	}
	case 0xB4:		//RES 6, H
	{
		//reset bit 6 in register H
		registers.h &= 0b10111111;
		registers.pc++;
		return 8;
	}
	case 0xB5:		//RES 6, L
	{
		//reset bit 6 in register L
		registers.l &= 0b10111111;
		registers.pc++;
		return 8;
	}
	case 0xB6:		//RES 6, (HL)
	{
		//reset bit 6 in register (HL)
		MEM->andWrite(registers.hl, 0b10111111);
		registers.pc++;
		return 16;
	}
	case 0xB7:		//RES 6, A
	{
		//reset bit 6 in register A
		registers.a &= 0b10111111;
		registers.pc++;
		return 8;
	}
	case 0xB8:		//RES 7, B
	{
		//reset bit 7 in register B
		registers.b &= 0b01111111;
		registers.pc++;
		return 8;
	}
	case 0xB9:		//RES 7, C
	{
		//reset bit 3 in register C
		registers.c &= 0b01111111;
		registers.pc++;
		return 8;
	}
	case 0xBA:		//RES 7, D
	{
		//reset bit 7 in register D
		registers.d &= 0b01111111;
		registers.pc++;
		return 8;
	}
	case 0xBB:		//RES 7, E
	{
		//reset bit 7 in register E
		registers.e &= 0b01111111;
		registers.pc++;
		return 8;
	}
	case 0xBC:		//RES 7, H
	{
		//reset bit 7 in register H
		registers.h &= 0b01111111;
		registers.pc++;
		return 8;
	}
	case 0xBD:		//RES 7, L
	{
		//reset bit 7 in register L
		registers.l &= 0b01111111;
		registers.pc++;
		return 8;
	}
	case 0xBE:		//RES 7, (HL)
	{
		//reset bit 7 in register (HL)
		MEM->andWrite(registers.hl, 0b01111111);
		registers.pc++;
		return 16;
	}
	case 0xBF:		//RES 7, A
	{
		//reset bit 7 in register A
		registers.a &= 0b01111111;
		registers.pc++;
		return 8;
	}
	case 0xC0:		//SET 0, B
	{
		//set bit 0 in register B
		registers.b |= 0b00000001;
		registers.pc++;
		return 8;
	}
	case 0xC1:		//SET 0, C
	{
		//set bit 0 in register C
		registers.c |= 0b00000001;
		registers.pc++;
		return 8;
	}
	case 0xC2:		//SET 0, D
	{
		//set bit 0 in register D
		registers.d |= 0b00000001;
		registers.pc++;
		return 8;
	}
	case 0xC3:		//SET 0, E
	{
		//set bit 0 in register E
		registers.e |= 0b00000001;
		registers.pc++;
		return 8;
	}
	case 0xC4:		//SET 0, H
	{
		//set bit 0 in register H
		registers.h |= 0b00000001;
		registers.pc++;
		return 8;
	}
	case 0xC5:		//SET 0, L
	{
		//set bit 0 in register L
		registers.l |= 0b00000001;
		registers.pc++;
		return 8;
	}
	case 0xC6:		//SET 0, (HL)
	{
		//set bit 0 in register (HL)
		MEM->orWrite(registers.hl, 0b00000001);
		registers.pc++;
		return 16;
	}
	case 0xC7:		//SET 0, A
	{
		//set bit 0 in register A
		registers.a |= 0b00000001;
		registers.pc++;
		return 8;
	}
	case 0xC8:		//SET 1, B
	{
		//set bit 1 in register B
		registers.b |= 0b00000010;
		registers.pc++;
		return 8;
	}
	case 0xC9:		//SET 1, C
	{
		//set bit 1 in register C
		registers.c |= 0b00000010;
		registers.pc++;
		return 8;
	}
	case 0xCA:		//SET 1, D
	{
		//set bit 1 in register D
		registers.d |= 0b00000010;
		registers.pc++;
		return 8;
	}
	case 0xCB:		//SET 1, E
	{
		//set bit 1 in register E
		registers.e |= 0b00000010;
		registers.pc++;
		return 8;
	}
	case 0xCC:		//SET 1, H
	{
		//set bit 1 in register H
		registers.h |= 0b00000010;
		registers.pc++;
		return 8;
	}
	case 0xCD:		//SET 1, L
	{
		//set bit 1 in register L
		registers.l |= 0b00000010;
		registers.pc++;
		return 8;
	}
	case 0xCE:		//SET 1, (HL)
	{
		//set bit 1 in register (HL)
		MEM->orWrite(registers.hl, 0b00000010);
		registers.pc++;
		return 16;
	}
	case 0xCF:		//SET 1, A
	{
		//set bit 1 in register A
		registers.a |= 0b00000010;
		registers.pc++;
		return 8;
	}
	case 0xD0:		//SET 2, B
	{
		//set bit 2 in register B
		registers.b |= 0b00000100;
		registers.pc++;
		return 8;
	}
	case 0xD1:		//SET 2, C
	{
		//set bit 2 in register C
		registers.c |= 0b00000100;
		registers.pc++;
		return 8;
	}
	case 0xD2:		//SET 2, D
	{
		//set bit 2 in register D
		registers.d |= 0b00000100;
		registers.pc++;
		return 8;
	}
	case 0xD3:		//SET 2, E
	{
		//set bit 2 in register E
		registers.e |= 0b00000100;
		registers.pc++;
		return 8;
	}
	case 0xD4:		//SET 2, H
	{
		//set bit 2 in register H
		registers.h |= 0b00000100;
		registers.pc++;
		return 8;
	}
	case 0xD5:		//SET 2, L
	{
		//set bit 2 in register L
		registers.l |= 0b00000100;
		registers.pc++;
		return 8;
	}
	case 0xD6:		//SET 2, (HL)
	{
		//set bit 2 in register (HL)
		MEM->orWrite(registers.hl, 0b00000100);
		registers.pc++;
		return 16;
	}
	case 0xD7:		//SET 2, A
	{
		//set bit 2 in register A
		registers.a |= 0b00000100;
		registers.pc++;
		return 8;
	}
	case 0xD8:		//SET 3, B
	{
		//set bit 3 in register B
		registers.b |= 0b00001000;
		registers.pc++;
		return 8;
	}
	case 0xD9:		//SET 3, C
	{
		//set bit 3 in register C
		registers.c |= 0b00001000;
		registers.pc++;
		return 8;
	}
	case 0xDA:		//SET 3, D
	{
		//set bit 3 in register D
		registers.d |= 0b00001000;
		registers.pc++;
		return 8;
	}
	case 0xDB:		//SET 3, E
	{
		//set bit 3 in register E
		registers.e |= 0b00001000;
		registers.pc++;
		return 8;
	}
	case 0xDC:		//SET 3, H
	{
		//set bit 3 in register H
		registers.h |= 0b00001000;
		registers.pc++;
		return 8;
	}
	case 0xDD:		//SET 3, L
	{
		//set bit 3 in register L
		registers.l |= 0b00001000;
		registers.pc++;
		return 8;
	}
	case 0xDE:		//SET 3, (HL)
	{
		//set bit 3 in register (HL)
		MEM->orWrite(registers.hl, 0b00001000);
		registers.pc++;
		return 16;
	}
	case 0xDF:		//SET 3, A
	{
		//set bit 3 in register A
		registers.a |= 0b00001000;
		registers.pc++;
		return 8;
	}
	case 0xE0:		//SET 4, B
	{
		//set bit 4 in register B
		registers.b |= 0b00010000;
		registers.pc++;
		return 8;
	}
	case 0xE1:		//SET 4, C
	{
		//set bit 4 in register C
		registers.c |= 0b00010000;
		registers.pc++;
		return 8;
	}
	case 0xE2:		//SET 4, D
	{
		//set bit 4 in register D
		registers.d |= 0b00010000;
		registers.pc++;
		return 8;
	}
	case 0xE3:		//SET 4, E
	{
		//set bit 4 in register E
		registers.e |= 0b00010000;
		registers.pc++;
		return 8;
	}
	case 0xE4:		//SET 4, H
	{
		//set bit 4 in register H
		registers.h |= 0b00010000;
		registers.pc++;
		return 8;
	}
	case 0xE5:		//SET 4, L
	{
		//set bit 4 in register L
		registers.l |= 0b00010000;
		registers.pc++;
		return 8;
	}
	case 0xE6:		//SET 4, (HL)
	{
		//set bit 4 in register (HL)
		MEM->orWrite(registers.hl, 0b00010000);
		registers.pc++;
		return 16;
	}
	case 0xE7:		//SET 4, A
	{
		//set bit 4 in register A
		registers.a |= 0b00010000;
		registers.pc++;
		return 8;
	}
	case 0xE8:		//SET 5, B
	{
		//set bit 5 in register B
		registers.b |= 0b00100000;
		registers.pc++;
		return 8;
	}
	case 0xE9:		//SET 5, C
	{
		//set bit 5 in register C
		registers.c |= 0b00100000;
		registers.pc++;
		return 8;
	}
	case 0xEA:		//SET 5, D
	{
		//set bit 5 in register D
		registers.d |= 0b00100000;
		registers.pc++;
		return 8;
	}
	case 0xEB:		//SET 5, E
	{
		//set bit 5 in register E
		registers.e |= 0b00100000;
		registers.pc++;
		return 8;
	}
	case 0xEC:		//SET 5, H
	{
		//set bit 5 in register H
		registers.h |= 0b00100000;
		registers.pc++;
		return 8;
	}
	case 0xED:		//SET 5, L
	{
		//set bit 5 in register L
		registers.l |= 0b00100000;
		registers.pc++;
		return 8;
	}
	case 0xEE:		//SET 5, (HL)
	{
		//set bit 5 in register (HL)
		MEM->orWrite(registers.hl, 0b00100000);
		registers.pc++;
		return 16;
	}
	case 0xEF:		//SET 5, A
	{
		//set bit 5 in register A
		registers.a |= 0b00100000;
		registers.pc++;
		return 8;
	}
	case 0xF0:		//SET 6, B
	{
		//set bit 6 in register B
		registers.b |= 0b01000000;
		registers.pc++;
		return 8;
	}
	case 0xF1:		//SET 6, C
	{
		//set bit 6 in register C
		registers.c |= 0b01000000;
		registers.pc++;
		return 8;
	}
	case 0xF2:		//SET 6, D
	{
		//set bit 6 in register D
		registers.d |= 0b01000000;
		registers.pc++;
		return 8;
	}
	case 0xF3:		//SET 6, E
	{
		//set bit 6 in register E
		registers.e |= 0b01000000;
		registers.pc++;
		return 8;
	}
	case 0xF4:		//SET 6, H
	{
		//set bit 6 in register H
		registers.h |= 0b01000000;
		registers.pc++;
		return 8;
	}
	case 0xF5:		//SET 6, L
	{
		//set bit 6 in register L
		registers.l |= 0b01000000;
		registers.pc++;
		return 8;
	}
	case 0xF6:		//SET 6, (HL)
	{
		//set bit 6 in register (HL)
		MEM->orWrite(registers.hl, 0b01000000);
		registers.pc++;
		return 16;
	}
	case 0xF7:		//SET 6, A
	{
		//set bit 6 in register A
		registers.a |= 0b01000000;
		registers.pc++;
		return 8;
	}
	case 0xF8:		//SET 7, B
	{
		//set bit 7 in register B
		registers.b |= 0b10000000;
		registers.pc++;
		return 8;
	}
	case 0xF9:		//SET 7, C
	{
		//set bit 3 in register C
		registers.c |= 0b10000000;
		registers.pc++;
		return 8;
	}
	case 0xFA:		//SET 7, D
	{
		//set bit 7 in register D
		registers.d |= 0b10000000;
		registers.pc++;
		return 8;
	}
	case 0xFB:		//SET 7, E
	{
		//set bit 7 in register E
		registers.e |= 0b10000000;
		registers.pc++;
		return 8;
	}
	case 0xFC:		//SET 7, H
	{
		//set bit 7 in register H
		registers.h |= 0b10000000;
		registers.pc++;
		return 8;
	}
	case 0xFD:		//SET 7, L
	{
		//set bit 7 in register L
		registers.l |= 0b10000000;
		registers.pc++;
		return 8;
	}
	case 0xFE:		//SET 7, (HL)
	{
		//set bit 7 in register (HL)
		MEM->orWrite(registers.hl, 0b10000000);
		registers.pc++;
		return 16;
	}
	case 0xFF:		//SET 7, A
	{
		//set bit 7 in register A
		registers.a |= 0b10000000;
		registers.pc++;
		return 8;
	}
	default:
	{
		Failure(2);
		return 0;
	}
	}
}

void gbCPU::timers(uint8_t clock) {
	static uint32_t clkCount = 0;
	if (dMEM[0xFF07] & 0b00000100) {
		clkCount += clock;
		int cycles = 0;
		switch (dMEM[0xFF07] & 0b00000011)
		{
		case 0:
			cycles = 1024;
			break;
		case 1:
			cycles = 16;
			break;
		case 2:
			cycles = 64;
			break;
		case 3:
			cycles = 256;
			break;
		}
		while(clkCount >= cycles) {
			dMEM[0xFF05]++;
			if (dMEM[0xFF05] == 0) {
				//overflow, reset, and request interrupt
				dMEM[0xFF05] = dMEM[0xFF06]; 
				dMEM[0xFF0F] |= 0b00000100;
			}
			clkCount -= cycles;
		}
	}
}

int gbCPU::interrupts(int cycles) {
	int IntCycles = 0; 
	//OAM DMA Transfer
	if (DMA != dMEM[0xFF46]) {
		DMA = dMEM[0xFF46];
		for (uint8_t byte = 0x00; byte < 0x9F; byte++) {
			dMEM[0xFE00 + byte] = dMEM[(DMA << 8) + byte];
		}
		dMEM[0xFF46] = 0xE1;
		DMA = 0xE1;
	}
	//DIV Register counting
	if (dMEM[0xFF04] != DIV) {
		DIV = 0x00;
		dMEM[0xFF04] = 0x00;
	}
	else {
		DIV+=cycles;
		dMEM[0xFF04]+=cycles;
	}
	if (IME) {
		if		(dMEM[0xFFFF] & 0b00000001 && dMEM[0xFF0F] & 0b00000001) {//Vblank
			dMEM[0xFF0F] &= 0b11111110;
			preIME = false;
			halted = false;
			IntCycles += 10;
			PushStack(((registers.pc) & 0xFF00) >> 8);
			PushStack((registers.pc) & 0x00FF);
			registers.pc = 0x0040;
			// printf("Vblank interrupt taken\n");
		}
		else if (dMEM[0xFFFF] & 0b00000010 && dMEM[0xFF0F] & 0b00000010) {//LCD STAT
			dMEM[0xFF0F] &= 0b11111101;
			preIME = false;
			halted = false; 
			IntCycles += 10;
			PushStack(((registers.pc) & 0xFF00) >> 8);
			PushStack((registers.pc) & 0x00FF);
			registers.pc = 0x0048;
			// printf("STAT interrupt taken\n");
		}
		else if (dMEM[0xFFFF] & 0b00000100 && dMEM[0xFF0F] & 0b00000100) {//Timer
			dMEM[0xFF0F] &= 0b11111011;
			preIME = false;
			halted = false;
			IntCycles += 10;
			PushStack(((registers.pc) & 0xFF00) >> 8);
			PushStack((registers.pc) & 0x00FF);
			registers.pc = 0x0050;
			// printf("TIMER interrupt taken\n");
		}
		else if (dMEM[0xFFFF] & 0b00001000 && dMEM[0xFF0F] & 0b00001000) {//Serial
			Failure(4);
			//yeah. actually. skip this. not for my emulator yet
		}
		else if (dMEM[0xFFFF] & 0b00010000 && dMEM[0xFF0F] & 0b00010000) {//Joypad
			Failure(4);
		}
	}
	else if (halted) {
		if (dMEM[0xFFFF] & 0b00000001 && dMEM[0xFF0F] & 0b00000001) {//Vblank
			halted = false;
		}
		else if (dMEM[0xFFFF] & 0b00000010 && dMEM[0xFF0F] & 0b00000010) {//LCD STAT
			halted = false;
		}
		else if (dMEM[0xFFFF] & 0b00000100 && dMEM[0xFF0F] & 0b00000100) {//Timer
			halted = false;
			IntCycles += 10;
		}
		else if (dMEM[0xFFFF] & 0b00001000 && dMEM[0xFF0F] & 0b00001000) {//Serial
			Failure(4);
		}
		else if (dMEM[0xFFFF] & 0b00010000 && dMEM[0xFF0F] & 0b00010000) {//Joypad
			Failure(4);
		}
	}
	if (preIME != IME) {
		IME = preIME;
		if (IME) {
			dMEM[0xFF0F] &= 0b11100000;
		}
	}
	return IntCycles;
}

void gbCPU::initCpu() {
	registers = { {0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00},0xFFFE,0x0100 };
	IME = 0; //Interrupt Master Enable
	preIME = 0; //used because IME only returns after one instruction
	halted = 0; //Shows if the CPU is Halted
	DMA = 0xE1;
	DIV = 0x00;
	lineprogress = 0;


	myfile.open ("example.txt");
}

void gbCPU::PushStack(uint8_t data) {
    registers.sp--;
    dMEM[registers.sp] = data;
}

uint8_t gbCPU::PopStack() {
    uint8_t data = dMEM[registers.sp];
    registers.sp++;
    return data;
}

void gbCPU::setZ(bool set) {
    if (set) {
        registers.f |= ZMASK;
    }
    else {
        registers.f &= ~ZMASK;
    }
}

void gbCPU::setN(bool set) {
    if (set) {
        registers.f |= NMASK;
    }
    else {
        registers.f &= ~NMASK;
    }
}

void gbCPU::setH(bool set) {
    if (set) {
        registers.f |= HMASK;
    }
    else {
        registers.f &= ~HMASK;
    }
}

void gbCPU::setC(bool set) {
    if (set) {
        registers.f |= 0b00010000;
    }
    else {
        registers.f &= 0b11101111;
    }
}

void gbCPU::Failure(int code) {switch (code) {
	case 0:
		std::cout << "Failed Instruction! Code: 0x";
		printf("%.2X", dMEM[registers.pc]);
		printf("\n");
		std::cout << ("PC: 0x");
		printf("%.4X", registers.pc);
		printf("\n");
		break;
	case 1:
		std::cout << "Hit invalid Opcode!" << std::endl;
		std::cout << ("PC: 0x");
		std::cout << std::hex << registers.pc << std::endl;
		break;
	case 2:
		std::cout << "Failed CB Instruction! Code: 0xCB 0x";
		printf("%.2X", dMEM[registers.pc]);
		printf("\n");
		std::cout << ("PC: 0x");
		printf("%.4X", registers.pc-1);
		printf("\n");
		break;
	case 3:
		std::cout << "HALT!" << std::endl;
		std::cout << ("PC: 0x");
		std::cout << std::hex << registers.pc << std::endl;
		break;
	case 4:
		std::cout << "Failed to fulfill requested Interrupt" << std::endl;
		std::cout << ("PC: 0x");
		std::cout << std::hex << registers.pc << std::endl;
		break;
    default:
        printf("unknown error\n");
	}
}

void gbCPU::printInstruction()
{
    // printf("PC-0x%04X IR-0x%02X  |  AF-0x%04X BC-0x%04X DE-0x%04X HL-0x%04X \n",registers.pc, dMEM[registers.pc], registers.af, registers.bc, registers.de, registers.hl);
	
	myfile << std::hex << registers.pc << " - " ;
	myfile << std::hex << (int)dMEM[registers.pc] << std::endl;
}
