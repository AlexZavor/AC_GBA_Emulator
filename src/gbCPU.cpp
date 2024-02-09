#include "gbCPU.h"

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
		Failure(0);
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
		// halted = true;
		Failure(3);
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
		// preIME = 1;
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
		// preIME = 0;
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
		// preIME = 1;
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
    dMEM[registers.pc];
    Failure(2);
    return 4;
}

void gbCPU::timers() {
}

void gbCPU::initCpu() {
	registers = { {0x00,0x00},{0x00,0x00},{0x00,0x00},{0x00,0x00},0xFFFE,0x0100 };
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
		printf("%.2X", MEM[registers.pc]);
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
		printf("%.2X", MEM[registers.pc]);
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
    printf("PC-0x%04X IR-0x%02X  |  AF-0x%04X BC-0x%04X DE-0x%04X HL-0x%04X \n",registers.pc, dMEM[registers.pc], registers.af, registers.bc, registers.de, registers.hl);
}
