#include "gba/gbaCPU.h"
#include "gba/gbaMEM.h"
#include "gba/bitwise.h"
#include "stdio.h"

extern int print;

typedef enum{
  AND = 0x0,
  EOR = 0x1,
  SUB = 0x2,
  RSB = 0x3,
  ADD = 0x4,
  ADC = 0x5,
  SBC = 0x6,
  RSC = 0x7,
  TST = 0x8,
  TEQ = 0x9,
  CMP = 0xA,
  CMN = 0xB,
  ORR = 0xC,
  MOV = 0xD,
  BIC = 0xE,
  MVN = 0xF,
} Op;

typedef enum{
  LSL = 0,
  LSR = 1,
  ASR = 2,
  ROR = 3,
  RCR = 4,
} Shift_t;

typedef enum{
/*10000b 10h 16 - User (non-privileged)
  10001b 11h 17 - FIQ
  10010b 12h 18 - IRQ
  10011b 13h 19 - Supervisor (SWI)
  10111b 17h 23 - Abort
  11011b 1Bh 27 - Undefined
  11111b 1Fh 31 - System (privileged 'User' mode) (ARMv4 and up)
*/
  USER =    0b10000,
  FIQ =     0b10001,
  IRQ =     0b10010,
  SUPER=    0b10011,
  ABORT=    0b10111,
  UNDEFINED=0b11011,
  SYSTEM=   0b11111,
} User_mode_t;

typedef struct{
  uint32_t 
    R8_fiq, R9_fiq, R10_fiq, R11_fiq, R12_fiq, R13_fiq, R14_fiq,
    R13_svc, R14_svc,
    R13_abt, R14_abt,
    R13_irq, R14_irq,
    R13_und, R14_und,
    SPSR_fiq, SPSR_svc, SPSR_abt, SPSR_irq, SPSR_und;
} User_mode_register_banks;


static uint32_t registers[16]; // 16 registers for now (theres also some mode banked ones)
#define PC (registers[15])
#define LR (registers[14])
#define SP (registers[13])
uint32_t CPSR = 0x2000001F;
uint32_t SPSR;
#define N_FLAG ((CPSR >> 31) & 0x1)
#define Z_FLAG ((CPSR >> 30) & 0x1)
#define C_FLAG ((CPSR >> 29) & 0x1)
#define V_FLAG ((CPSR >> 28) & 0x1)
User_mode_t usr_mode = USER;
User_mode_register_banks usr_mode_banks;

static bool THUMB = false; // ARM or Thumb mode

void gbaCPU_init(){
    PC = 0x08000000; // start of cartridge
    LR = 0x08000000;
    SP = 0x03007F00;
    CPSR = 0x2000001F;
    THUMB = false; // Starts in ARM mode
}
void gbaCPU_deinit(){}

// Called to print errors
static void Failure(int code) {
	switch (code) {
	case 0:
		printf("Failed Instruction! Code: 0x%.8X\n \tPC: 0x%.8X\n", gba_read32(PC), PC);
		break;
	case 1:
		printf("Hit invalid Opcode! Code: 0x%.8X\n \tPC: 0x%.8X\n", gba_read32(PC), PC);
		break;
	case 2:
		printf("Unimplemented Condition! Code: 0x%.8X\n \tPC: 0x%.8X\n", gba_read32(PC), PC);
		break;
	case 3:
		printf("Block Transfer Error! Code: 0x%.8X\n \tPC: 0x%.8X\n", gba_read32(PC), PC);
		break;
	case 4:
		printf("Data Proc Error! Code: 0x%.8X\n \tPC: 0x%.8X\n", gba_read32(PC), PC);
		break;
	case 5:
		printf("Thumb Instr. Error! Code: 0x%.4X\n \tPC: 0x%.8X\n", gba_read16(PC), PC);
		break;
  default:
      printf("unknown error\n");
	}
}

// Set N and Z Flag based on input data
static inline void NZ_check(uint32_t data){
  CPSR &= ~(0x3<<30);
  if(data == 0){CPSR |= (0x1<<30);}
  if(data & 0x1<<31){CPSR |= (0x1<<31);}
}

// Returns true if condition is met, false otherwise
static inline bool condition(uint8_t cond){
/*
  Code Suffix Flags         Meaning
  0:   EQ     Z=1           equal (zero) (same)
  1:   NE     Z=0           not equal (nonzero) (not same)
  2:   CS/HS  C=1           unsigned higher or same (carry set)
  3:   CC/LO  C=0           unsigned lower (carry cleared)
  4:   MI     N=1           signed negative (minus)
  5:   PL     N=0           signed positive or zero (plus)
  6:   VS     V=1           signed overflow (V set)
  7:   VC     V=0           signed no overflow (V cleared)
  8:   HI     C=1 and Z=0   unsigned higher
  9:   LS     C=0 or Z=1    unsigned lower or same
  A:   GE     N=V           signed greater or equal
  B:   LT     N<>V          signed less than
  C:   GT     Z=0 and N=V   signed greater than
  D:   LE     Z=1 or N<>V   signed less or equal
  E:   AL     -             always (the "AL" suffix can be omitted)
  F:   NV     -             never (ARMv1,v2 only) (Reserved ARMv3 and up)
*/
  switch (cond)
  {
  case 0x00:
    return Z_FLAG;
  case 0x01:
    return !Z_FLAG;
  case 0x02:
    return C_FLAG;
  case 0x03:
    return !C_FLAG;
  case 0x04:
    return N_FLAG;
  case 0x05:
    return !N_FLAG;
  case 0x06:
    return V_FLAG;
  case 0x07:
    return !V_FLAG;
  case 0x08:
    return C_FLAG && !Z_FLAG;
  case 0x09:
    return (!C_FLAG) | Z_FLAG;
  case 0x0A:
    return N_FLAG == V_FLAG;
  case 0x0B:
    return N_FLAG != V_FLAG;
  case 0x0C:
    return (!Z_FLAG) && (N_FLAG == V_FLAG);
  case 0x0D:
    return (Z_FLAG) || (N_FLAG != V_FLAG);
  case 0x0E:
    return true;
  
  default:
    Failure(2);
    exit(1);
    return false;
  }
}

// Sets CPSR data masked
void setCPSR(uint32_t data, uint32_t mask){
  CPSR &= ~mask;
  CPSR |= (data & mask);
}
void setSPSR(uint32_t data, uint32_t mask){
  SPSR &= ~mask;
  SPSR |= (data & mask);
}

// Sets operating mode
void setMode(User_mode_t mode){
  if(mode == usr_mode){return;}
  User_mode_t old_mode = usr_mode;
  if(mode == FIQ || old_mode == FIQ){
    swp(&registers[8], &usr_mode_banks.R8_fiq);
    swp(&registers[9], &usr_mode_banks.R9_fiq);
    swp(&registers[10], &usr_mode_banks.R10_fiq);
    swp(&registers[11], &usr_mode_banks.R11_fiq);
    swp(&registers[12], &usr_mode_banks.R12_fiq);
  }
  SPSR = CPSR;
  usr_mode = mode;
  //TODO: set Mode
}

// Thumb transfer function
int8_t thumb_instr();

#define LSL(x,y) ((uint32_t)(x)<<(uint32_t)(y))
#define LSR(x,y) ((x) >> (y))
#define ASR(x,y) ((int32_t)(x) >> (y))
#define ROR(x,y) ((uint32_t)(x) >> (y) | (uint32_t)(x) << (32 - (y)))

uint32_t shift(uint32_t data, uint32_t shift, Shift_t type, bool setCflag, bool IMM){
  bool old_c = C_FLAG;
  if(setCflag){
    switch (type){
    case LSL:
      if(shift == 0 && IMM){break;}
      set_bit(&CPSR, 29, (data<<(shift-1))&0x80000000);
      break;
    case LSR:
      if(shift == 0 && IMM){
        // Set carry flag to bit 31 of data and return 0
        set_bit(&CPSR, 29, (data>>(31)));
        return 0;
      }
      if(shift != 0){
        set_bit(&CPSR, 29, (data>>(shift-1))&0x1);
      }
      break;
    case ASR:
      if(shift == 0 && IMM){
        // Set carry flag and output to msb of data
        if((data>>31)){
          CPSR |= ((0x1<<29));
          return UINT32_MAX;
        }else{
          CPSR &= ~((0x1<<29));
          return 0;
        }
      }
      if(shift != 0)
        set_bit(&CPSR, 29, (data>>(shift-1))&0x1);
      break;
    case ROR:
      if(shift == 0){
        break;
      }
      set_bit(&CPSR, 29, (data>>(shift-1))&1);
      break;
    case RCR:
      set_bit(&CPSR, 29, (data&1));
      break;
    }
  }
  switch (type){
  case LSL:
    if(shift >= 32){return 0;}
    return LSL(data, shift);
  case LSR:
    if(shift >= 32){return 0;}
    return LSR(data, shift);
  case ASR:
    if(shift >= 32){return (((int32_t)data)<0)?-1:1;}
    return ASR(data, shift);
  case ROR:
    return ROR(data, shift);
  case RCR:
    return (data >>1) | (old_c<<31);
  }
  return -1;
}

// Performs All Transfer Operations
// Returns Num of Cycles or -1 if error
uint32_t transfer(uint32_t instr, uint8_t Rd, uint8_t Rn, uint8_t Rm){
  /*
X |_Cond__|0_0_0|P|U|0|W|L|__Rn___|__Rd___|0_0_0_0|1|S|H|1|__Rm___| TransReg10
X |_Cond__|0_0_0|P|U|1|W|L|__Rn___|__Rd___|OffsetH|1|S|H|1|OffsetL| TransImm10
  |_Cond__|0_1_0|P|U|B|W|L|__Rn___|__Rd___|_________Offset________| TransImm9
  |_Cond__|0_1_1|P|U|B|W|L|__Rn___|__Rd___|__Shift__|Typ|0|__Rm___| TransReg9
  |_Cond__|0_1_1|________________xxx____________________|1|__xxx__| Undefined
  */
  bool P = instr & 0x01000000;
  bool U = instr & 0x00800000;
  bool B = instr & 0x00400000;
  bool W = instr & 0x00200000;
  bool L = instr & 0x00100000;
  if(!P && W){printf("Invalid Trans Format\n");Failure(0);return-1;}
  PC += 4; // next instruction
  uint32_t addr = registers[Rn] + ((Rn==15)?4:0);
  uint32_t offset;
  if(instr & 0x04000000){//Trans 9
    bool Reg = instr & 0x02000000;
    if(Reg){
      if((instr>>4)&0x1){printf("Undefined Function Err\n");Failure(0);return-1;}
      // TransReg9
      uint32_t Shift;
      uint32_t Data = registers[instr&0xF];
      if((instr&0xF) == 15){
        // PC special
        Data += 4;
      }
      Shift = (instr & 0x00000F80)>>7;
      Shift_t Shift_typ = (instr & 0x00000060)>>5;
      if(Shift == 0){
        if(Shift_typ == ROR){// Special case
          Shift_typ = RCR;
          Shift = 1;
        }
      }
      offset = shift(Data, Shift, Shift_typ, false, false);
    }else{
      // TransImm9
      offset = instr & 0xFFF;
    }
    if(P){addr += (offset*(U?1:-1));}
    if(L){// Load
      if(B){
        registers[Rd] = gba_read8(addr);
      }
      else{
        registers[Rd] = gba_read32(addr-(addr%4));
        if(addr%4)
          registers[Rd] = ROR(registers[Rd], (addr%4)*8);
      }
    }else{// Store
      if(B){
        gba_write8(addr, registers[Rd]);
      }else
        gba_write32(addr-(addr%4), registers[Rd] + ((Rd==15)?8:0));
    }
  }else{// Trans10
    uint8_t mode = (instr & 0x60)>>5;
    if((instr >> 22)&0x01){
      // TransImm10
      offset = ((instr>>4)&0xF0) + ((instr)&0xF);
    }else{
      // TransReg10
      if((instr & 0xF00) != 0){printf("Invalid Trans Format\n");Failure(0);return-1;}
      offset = registers[instr&0xF];
    }
    if(P){addr += (offset*(U?1:-1));}
    if(L){//load
      /*
        1: LDR{cond}H  Rd,<Address>  ;Load Unsigned halfword (zero-extended)
        2: LDR{cond}SB Rd,<Address>  ;Load Signed byte (sign extended)
        3: LDR{cond}SH Rd,<Address>  ;Load Signed halfword (sign extended)*/
      switch (mode){
      case 1:// Load unsigned Halfword
        registers[Rd] = gba_read16(addr);
        if(addr%2)
          registers[Rd] = ROR(registers[Rd], (addr%2)*8);
        break;
      case 2:// Load signed Byte
        registers[Rd] = sign_extend(gba_read8(addr),8);
        break;
      case 3:// Load signed Halfword
        registers[Rd] = sign_extend(gba_read16(addr),16);
        if(addr%2){
          registers[Rd] = ROR(registers[Rd], (addr%2)*8);
          registers[Rd] = sign_extend(registers[Rd], 24);
        }
        break;
      default:
        printf("Load unimplemented Mode %X\n", mode);
        return -1;
        break;
      }
    }else{//store
      /*
        1: STR{cond}H  Rd,<Address>  ;Store halfword   [a]=Rd
        2: LDR{cond}D  Rd,<Address>  ;Load Doubleword  R(d)=[a], R(d+1)=[a+4]
        3: STR{cond}D  Rd,<Address>  ;Store Doubleword [a]=R(d), [a+4]=R(d+1)
      */
      switch (mode){
      case 1:// Store Halfword
        // printf("halfword - %.8X\n", registers[Rd]);
        gba_write16(addr-(addr%2), registers[Rd]);
        break;
      
      default:
        printf("Store unimplemented Mode %X\n", mode);
        return -1;
        break;
      }
    }
  }
  if((Rn != Rd) || (!L)){
    if(!P){
      addr += (offset*(U?1:-1));
      registers[Rn] = addr;
    }
    if(W){
      registers[Rn] = addr;
    }
  }
  return 1;// 1S + 1N + 1I
}


// Does other operations that match Data Processing syntax.
// Returns num of cycles or -1 if error
uint32_t Data_proc_alt(uint32_t instr, uint8_t Rd, uint8_t Rn, uint8_t Rm){
  /*
  |..3 ..................2 ..................1 ..................0|
  |1_0_9_8_7_6_5_4_3_2_1_0_9_8_7_6_5_4_3_2_1_0_9_8_7_6_5_4_3_2_1_0|
X |_Cond__|0_0_0|___Op__|S|__Rn___|__Rd___|__Rs___|0|Typ|1|__Rm___| DataProc
   X X X X 0 0 0 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 1 _ _ 1 _ _ _ _
  |_Cond__|0_0_0_0_0_0|A|S|__Rd___|__Rn___|__Rs___|1_0_0_1|__Rm___| Multiply
  |_Cond__|0_0_0_0_0_1_0_0|_RdHi__|_RdLo__|__Rs___|1_0_0_1|__Rm___| ARM11:UMAAL
  |_Cond__|0_0_0_0_1|U|A|S|_RdHi__|_RdLo__|__Rs___|1_0_0_1|__Rm___| MulLong
  |_Cond__|0_0_0_1_0|B|0_0|__Rn___|__Rd___|0_0_0_0|1_0_0_1|__Rm___| TransSwp12
  |_Cond__|0_0_0_1_1|_Op__|__Rn___|__Rd___|1_1_1_1|1_0_0_1|__Rm___| ARM11:LDREX
X |_Cond__|0_0_0|P|U|0|W|L|__Rn___|__Rd___|0_0_0_0|1|S|H|1|__Rm___| TransReg10
X |_Cond__|0_0_0|P|U|1|W|L|__Rn___|__Rd___|OffsetH|1|S|H|1|OffsetL| TransImm10
  */
  if((instr & 0x60) != 0x00){
    return transfer(instr, Rd, Rn, Rm);
  }
  if((instr & 0x0FC00000) == 0){
    bool Accumulate = (instr >> 21)&1;
    bool S = (instr >> 20)&1;
    swp(&Rd, &Rn);
    uint8_t Rs = (instr >> 8)&0xF;
    registers[Rd] = (registers[Rm] * registers[Rs]);
    if(Accumulate)
      registers[Rd] += registers[Rn];
    if(S)
      NZ_check(registers[Rd]);
    int m = ((abs(registers[Rs]))/8)+1; // close enough I think
    PC += 4;
    return 1 + (m+Accumulate);
  }
  if((instr & 0x0FC00000) == 0x00400000){//UMAAL
    uint8_t Rs = (instr >> 8)&0xF;
    uint64_t result = ((uint64_t)registers[Rm] * (uint64_t)registers[Rs]);
    registers[Rd] = result & UINT32_MAX;
    registers[Rn] = result >> 32;
    int m = ((abs(registers[Rs]))/8)+1; // close enough I think
    PC += 4;
    return 1 + (m);
  }
  if(((instr>>23)&0x1F) == 0x1){// MulLong
    bool Usign = get_bit(instr, 22);
    bool Acc = get_bit(instr, 21);
    bool S = get_bit(instr, 20);
    uint8_t Rs = (instr >> 8)&0xF;
    uint64_t result;
    if(!Usign){
      result = ((uint64_t)registers[Rm] * (uint64_t)registers[Rs]);
    }else{
      result = sign_extend64((uint64_t)registers[Rm], 32) * sign_extend64((uint64_t)registers[Rs], 32);
    }
    if(Acc){
      result += registers[Rd];
      result += ((uint64_t)registers[Rn])<<32;
    }
    registers[Rd] = (result & UINT32_MAX);
    registers[Rn] = (result >> 32);
    if(S)
      NZ_check(registers[Rn]);
    int m = ((abs(registers[Rs]))/8)+1; // close enough I think
    PC += 4;
    return 1 + (m);
  }
  printf("Data Proc Alt Failed\n");
  Failure(4);
  return -1;
}

// Block Data Transfer, return time taken, or -1 for error.
int8_t block_transfer(uint32_t instr){
  bool P = instr & 0x01000000;
  bool U = instr & 0x00800000;
  bool S = instr & 0x00400000;
  bool W = instr & 0x00200000;
  bool L = instr & 0x00100000;
  uint8_t Rn = (instr & 0x000F0000)>>16;
  uint32_t rlist = instr & 0xFFFF;
  int num_moved = high_count(rlist);
  uint32_t addr = (registers[Rn]-(registers[Rn]%4));
  uint32_t low_add = addr + ((!U)*(-num_moved*4));
  bool first = true;
  uint32_t base = 0;
  // printf("%c- 0x%.4X = %.8X %c%c\n", L?'L':'S', rlist, addr, U?'u':'d', P?'<':'>');
  if(num_moved == 0){// empty list
    // R15 loaded/stored (ARMv4 only), and Rb=Rb+/-40h (ARMv4-v5).
    registers[Rn] += 0x40 * (U?1:-1);
    if(L){
      registers[15] = gba_read32(addr);
    }else{
      gba_write32(addr+((0x3C) * (U?0:-1)) + ((P*0x4)*(U?1:-1)), registers[15] + 8);
    }
    return 1;
  }
  if(L){// load/pop
    int r = 0;
    int reads = 0;
    while (r<16){
      if(rlist & (0x1<<r)){
        if(P^(!U)) reads++;
        uint32_t offset = (reads*4);
        if(S){
          ((uint32_t*)&usr_mode_banks)[r-8] = gba_read32(low_add + offset);
        }else{
          registers[r] = gba_read32(low_add + offset);
          // printf("    r %d -> 0x%.8X\n", r, low_add + offset);
        }
        if(!P^(!U)) reads++;
        first = false;
      }
      r++;
    }
    if(W){//writeback
      if(!(instr&(1<<Rn))){// dont wb if updated
        registers[Rn] += reads*4*(U?1:-1);
      }
    }
    return reads;
  }else{// store/push
    int r = 0;
    int writes = 0;
    while (r<16){
      if(instr & (0x1<<r)){
        if(P^(!U)) writes++;
        uint32_t offset = (writes*4);
        if(S){// sys registers
          gba_write32(low_add + offset, ((uint32_t*)&usr_mode_banks)[r-8]);
        }else{// normal
          if(Rn == (r) && !first){
            base = low_add + offset;
          }
          gba_write32(low_add + offset, registers[r] + ((r==15)?8:0));
          // printf("    w %d -> 0x%.8X\n", r, low_add + offset);
        }
        if(!P^(!U)) writes++;
        first = false;
      }
      r++;
    }
    if(W){//writeback
      registers[Rn] += ((writes)*4) * (U?1:-1);
      if(base != 0){// re-write if not first
        gba_write32(base, registers[Rn]);
      }
    }
    return (writes-1);//(n-1)S+2N
  }
}

// Does other operations that fit in invalid ALU syntax (check op w/ no set flag).
// Returns num of cycles or -1 if error
uint32_t ALU_no_set(uint32_t instr){
  /*
   X X X X 0 0 _ 1 0 _ _ 0 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
  |_Cond__|0_0_1_1_0_0_1_0_0_0_0_0_1_1_1_1_0_0_0_0|_____Hint______| ARM11:Hint
  |_Cond__|0_0_1_1_0|P|1|0|_Field_|__Rd___|_Shift_|___Immediate___| PSR Imm
  |_Cond__|0_0_0_1_0|P|L|0|_Field_|__Rd___|0_0_0_0|0_0_0_0|__Rm___| PSR Reg
  |_Cond__|0_0_0_1_0_0_1_0_1_1_1_1_1_1_1_1_1_1_1_1|0_0|L|1|__Rn___| BX,BLX
  |1_1_1_0|0_0_0_1_0_0_1_0|_____immediate_________|0_1_1_1|_immed_| ARM9:BKPT
  |_Cond__|0_0_0_1_0_1_1_0_1_1_1_1|__Rd___|1_1_1_1|0_0_0_1|__Rm___| ARM9:CLZ
  |_Cond__|0_0_0_1_0|Op_|0|__Rn___|__Rd___|0_0_0_0|0_1_0_1|__Rm___| ARM9:QALU
  |_Cond__|0_0_0_1_0|Op_|0|Rd/RdHi|Rn/RdLo|__Rs___|1|y|x|0|__Rm___| MulHalfARM9
  |_Cond__|0_0_0_1_0|B|0_0|__Rn___|__Rd___|0_0_0_0|1_0_0_1|__Rm___| TransSwp12
  */
  if((instr & 0x0FFFFF00) == 0x0320F000){
    printf("Arm11: Hint?\n");
    if((instr&0xFF) == 0){PC+=4;return 1;}//Nop
    Failure(4);
    return -1;
  }else if(instr & 0x02000000 || ((instr & 0x00000FF0) == 0)){
    bool Imm = instr & 0x02000000;
    bool P = instr & 0x00400000;
    bool L = instr & 0x00200000;
    if(L){
      // Write Flags(MSR)
      bool F = get_bit(instr,19);
      bool S = (instr>>18)&1;
      bool X = (instr>>17)&1;
      bool C = (instr>>16)&1;
      uint32_t Mask = (F?(0xFF<<24):0) | (S?(0xFF<<16):0) | (X?(0xFF<<8):0) | (C?(0xFF):0);
      if(S||X){printf("Permission!\n");Failure(4);return-1;}
      if((instr>>12 & 0xF) != 0xF){printf("Write Flags(MSR) Format error!\n");Failure(4);return-1;} 
      uint32_t Op;
      if(Imm){
        uint32_t Shift = (instr>>8)&0xF;
        uint32_t Data = (instr&0xFF);
        Op = shift(Data, Shift*2, ROR, 0, 0);
      }else{
        if((instr>>4 & 0xFF) != 0x00){printf("Write Flags(MSR) Reg Format error!\n");Failure(4);return-1;} 
        Op = registers[instr&0xF];
      }
      if(P){
        setSPSR(Op, Mask);
      }else{
        setCPSR(Op, Mask);
        if(C){
          setMode(Op & 0x1F);
        }
      }
    }else{
      // Read Flags (MRS)
      //instr check
      if(((~(instr>>16))&0xF) != 0){
        printf("SWP?\n");
        Failure(4);return-1;
      }
      if((instr&0xFFF) != 0){
        printf("MRS format err\n");
        Failure(4);return-1;
      }
      uint8_t Rd = (instr>>12)&0xF;
      registers[Rd] = CPSR;
    }
    PC += 4; // next instruction
    return 1; // 1S 
  }else if((instr & 0x0FFFFF00) == 0x012FFF00){
    // BX/BLX
    uint32_t addr = registers[instr&0xF];
    bool L = (instr >> 5) & 0x1;
    if(L){
      LR = PC + 4;
    }
    if(addr&0x1){
      THUMB = true;
      addr--;
    }
    PC = addr;
    return 2; //2S + 1N
  }else if(((instr>>4)&0xF) == 9){// TransSwp12
    //Rd=[Rn], [Rn]=Rm
    bool B = get_bit(instr, 22);
    uint8_t Rm = instr&0xF;
    uint8_t Rd = (instr>>12)&0xF;
    uint8_t Rn = (instr>>16)&0xF;
    if(B){
      uint32_t data = gba_read8( registers[Rn] );
      if(registers[Rn]%2)
        data = ROR(data, (registers[Rn]%2)*8);
      gba_write8( registers[Rn]-(registers[Rn]%2), registers[Rm]);
      registers[Rd] = data;
    }else{
      uint32_t data = gba_read32( registers[Rn] );
      if(registers[Rn]%2)
        data = ROR(data, (registers[Rn]%2)*8);
      gba_write32( registers[Rn]-(registers[Rn]%2), registers[Rm]);
      registers[Rd] = data;
    }
    PC += 4;
    return 1; //1S+2N+1I
  }
  printf("check no set\n");
  Failure(4);
  return -1;
}

// Handles software interrupts by emulating behavior
int8_t software_int(uint8_t cmd){
  switch (cmd){
  case 0x01:{ // Ram Reset
    PC += 4;
    return 20;// meh. reset some memory
    //TODO Ram Reset?
  }
  case 0x05:{ // VblankInt wait??
    PC += 4;
    return 10;// no clue
    //TODO Vblank Wait
  }
  case 0x06:{ // div
    int32_t n = registers[0];
    int32_t d = registers[1];
    if(d == 0){printf("div by 0\n");return -1;}
    registers[0] = (n/d);
    registers[1] = (n%d);
    registers[3] = abs(n/d);
    PC += 4;
    return 4;// IDK
  }
  case 0x0C:{ // CPU Fast Set
    // R0 = Source
    // R1 = Dest
    // R2 = Word count (bit 24 - mode)
    if(get_bit(registers[2], 24)){
      gba_memset(registers[1], gba_read32(registers[0]), (registers[2]&0xFFFFF));
    }else{
      gba_memcpy(registers[1], registers[0], (registers[2]&0xFFFFF));
    }
    PC += 4;
    return 4;// IDK
  }
  default:
    printf("Software Int. failed - %.2X\n",cmd);
    return -1;
  }
}


//Returns number of cycles taken to execute or negative if error
int8_t gbaCPU_instruction(){
  set_bit(&CPSR, 5, THUMB);
  if(THUMB){ return thumb_instr(); }
  static int8_t time;
  uint32_t instr = gba_read32(PC);
  if(instr == 0xEAFFFFFE){return 1;}// infinite loop
  if(condition((instr & 0xF0000000)>>28)){
    switch((instr & 0x0E000000) >> 24){
      case 0x0:
      case 0x2:{// Data Processing (and a lot of others)
        bool I = instr & 0x02000000;
        Op op = (instr & 0x01E00000)>>21;
        bool S = instr & 0x00100000;
        uint8_t Rn = (instr & 0x000F0000)>>16;
        uint8_t Rd = (instr & 0x0000F000)>>12;
        if(op&0x8 && !(op&0x4) && !S){
          return ALU_no_set(instr);
        }
        uint32_t Op2;
        bool had_C = C_FLAG;
        if(I){// Immediate
          uint32_t Shift = (instr & 0x00000F00)>>8;
          Op2 = shift((instr & 0xFF), (Shift*2), ROR, S, false);
          // printf("i-%d", Op2);
        }else{
          uint32_t Shift;
          uint32_t Data = registers[instr&0xF];
          if((instr&0xF) == 15){
            // PC special
            Data += 8;
            // printf("data-%.8X\n",  Data);
          }
          bool IMM;
          if(!(instr & 0x10)){
            // imm shift
            Shift = (instr & 0x00000F80)>>7;
            IMM = true;
          }else{
            // Reg shift
            if(instr & 0x80){
              return Data_proc_alt(instr, Rd, Rn, instr&0xF);
            }
            uint32_t Rs = (instr & 0x00000F00)>>8;
            Shift = registers[Rs] & 0xFF;
            if(Rs == 15){Shift += 4; Shift&=0xFF;}
            IMM = false;
          }
          Shift_t Shift_typ = (instr & 0x00000060)>>5;
          if(Shift == 0 && IMM){
            if(Shift_typ == ROR){// Special case
              Shift_typ = RCR;
              Shift = 1;
            }
          }
          Op2 = shift(Data, Shift, Shift_typ, (S), IMM);
        }
        PC += 4; // next instruction
        uint32_t Op1 = registers[Rn] + ((Rn==15)?4:0);
        if(S){
          switch (op)
          {
          case ADD:
          case CMN:
            NZ_check(Op1 + Op2);
            set_bit(&CPSR, 29, (uint64_t) Op1 + (uint64_t) Op2 > UINT32_MAX);
            bool v_set = (Op1>0 && Op2>0 && N_FLAG) ||
                         (Op1<0 && Op2<0 && (!N_FLAG));
            set_bit(&CPSR, 28, v_set);
            break;
          case ADC:
            NZ_check(Op1 + Op2 + C_FLAG);
            set_bit(&CPSR, 29, (uint64_t) Op1 + (uint64_t) Op2 + C_FLAG > UINT32_MAX);
            v_set = (Op1>0 && Op2>0 && N_FLAG) ||
                         (Op1<0 && Op2<0 && (!N_FLAG));
            set_bit(&CPSR, 28, v_set);
            break;
          case SUB:
          case CMP:
            NZ_check(Op1 - Op2);
            set_bit(&CPSR, 29, (Op1 >= Op2));
            v_set = ((int32_t)Op1>0 && ((int32_t)Op2)<0 && N_FLAG) ||
                         ((int32_t)Op1<0 && ((int32_t)Op2)>0 && (!N_FLAG));
            set_bit(&CPSR, 28, v_set);
            break;
          case SBC:
            NZ_check(Op1 - Op2 + C_FLAG -1);
            set_bit(&CPSR, 29, (Op1+C_FLAG-1 >= Op2));
            v_set = ((int32_t)Op1>0 && (int32_t)Op2<0 && N_FLAG) ||
                         ((int32_t)Op1<0 && (int32_t)Op2>0 && (!N_FLAG));
            set_bit(&CPSR, 28, v_set);
            break;
          case TST:
            NZ_check(Op1 & Op2);
            break;
          case TEQ:
            NZ_check(Op1 ^ Op2);
            break;
          case MOV:
            NZ_check(Op2);
            break;
          default:
            printf("I don't know how to check Op -0x%.1X\n", op);
            Failure(4);
            return -1;
          }
        }
        switch (op){
        case AND:
          registers[Rd] = Op1 & Op2;
          break;
        case EOR:
          registers[Rd] = Op1 ^ Op2;
          break;
        case SUB:
          registers[Rd] = Op1 - Op2;
          break;
        case RSB:
          registers[Rd] = Op2 - Op1;
          break;
        case ADD:
          registers[Rd] = Op1 + Op2;
          break;
        case ADC:
          registers[Rd] = Op1 + Op2 + had_C;
          break;
        case SBC:
          registers[Rd] = Op1 - Op2 + had_C -1;
          break;
        case RSC:
          registers[Rd] = Op2 - Op1 + had_C -1;
          break;
        case TST:
        case TEQ:
        case CMP:
        case CMN:
          if ((Rd != 0x0) && (Rd != 0xF)){
            printf("Test Rd not 0\n");Failure(4);return-1;
          }
          break;
        case MOV:
          if(Rn != 0){Failure(4);return-1;}
          registers[Rd] = Op2;
          break;
        case ORR:
          registers[Rd] = registers[Rn] | Op2;
          break;
        case BIC:
          registers[Rd] = registers[Rn] & ~Op2;
          break;
        case MVN:
          if(Rn != 0){Failure(4);return-1;}
          registers[Rd] = ~Op2;
          break;
        default:
          printf("Undefined Op -0x%.1X\n", op);
          Failure(4);
          return -1;
        }

        break;
      }
      case 0x4:
      case 0x6:{// TransImm9
        uint32_t Rn = (instr & 0x000F0000)>>16;
        uint32_t Rd = (instr & 0x0000F000)>>12;
        uint32_t Rm = instr&0xF;
        time = transfer(instr, Rd, Rn, Rm);
        break;
      }
      case 0x8:{// Block Data Transfer
        PC += 4; // next instruction, done early so it can be overwritten
        time = block_transfer(instr);
        break;
      }
      case 0xA:{// Branch
        bool link = instr & 0x01000000;
        if(link){
          LR = PC+4;
        }
        uint32_t jump_24 = ((instr & 0x00FFFFFF));
        int32_t jump = ((jump_24&0x00800000)?0xFF000000:0) | jump_24;
        // printf("jump - %d\n",jump);
        PC += 8+((jump)*4); 
        time = 2; // 2S + 1N
        break;
      }
      case 0xE:{// Software Int and junk
        if((instr >> 24)&1){
          // Software Interrupt!
          return software_int((instr>>16)&0xFF);
        }
      }
      default:
        Failure(0);
        return -1;
    }
  }else{time = 1; PC += 4;} // Condition Failed
  return time;
}

// ================== THUMB Functions ==============

// Runs Thumb Instruction, returns the time taken
int8_t thumb_instr(){
  static int8_t time;
  uint16_t instr = gba_read16(PC);
  switch((instr & 0xE000) >> 13){
    case 0:{// 1-2
      uint8_t Op = (instr>>11)&0x3;
      if(Op != 3){// 1 - Shifted
        /*THUMB.1: move shifted register
          12-11  Opcode
                  00b: LSL{S} Rd,Rs,#Offset   (logical/arithmetic shift left)
                  01b: LSR{S} Rd,Rs,#Offset   (logical    shift right)
                  10b: ASR{S} Rd,Rs,#Offset   (arithmetic shift right)
                  11b: Reserved (used for add/subtract instructions)
          10-6   Offset                     (0-31)
        Example: LSL Rd,Rs,#nn ; Rd = Rs << nn ; ARM equivalent: MOVS Rd,Rs,LSL #nn
        Zero shift amount is having special meaning (same as for ARM shifts), LSL#0 performs no shift (the carry flag remains unchanged), LSR/ASR#0 are interpreted as LSR/ASR#32. Attempts to specify LSR/ASR#0 in source code are automatically redirected as LSL#0, and source LSR/ASR#32 is redirected as opcode LSR/ASR#0.
        Flags: Z=zeroflag, N=sign, C=carry (except LSL#0: C=unchanged), V=unchanged.*/
        uint8_t Shift = (instr>>6)&0x1F;
        uint8_t Rs = (instr>>3)&0x7;
        uint8_t Rd = (instr)&0x7;
        if(Shift == 0){
          // special
          registers[Rd] = shift(registers[Rs], Shift, Op, true, true);//???
        }else{
          registers[Rd] = shift(registers[Rs], Shift, Op, true, false);
        }
        NZ_check(registers[Rd]);
        PC += 2;
        return 1; // 1S
      }else{// 2 - Add/Sub
        /*
        THUMB.2: add/subtract
          10-9   Opcode (0-3)
                  0: ADD{S} Rd,Rs,Rn   ;add register        Rd=Rs+Rn
                  1: SUB{S} Rd,Rs,Rn   ;subtract register   Rd=Rs-Rn
                  2: ADD{S} Rd,Rs,#nn  ;add immediate       Rd=Rs+nn
                  3: SUB{S} Rd,Rs,#nn  ;subtract immediate  Rd=Rs-nn
                Pseudo/alias opcode with Imm=0:
                  2: MOV{ADDS} Rd,Rs   ;move (affects cpsr) Rd=Rs+0
          8-6    For Register Operand:
                  Rn - Register Operand (R0..R7)
                For Immediate Operand:
                  nn - Immediate Value  (0-7)
          5-3    Rs - Source register       (R0..R7)
          2-0    Rd - Destination register  (R0..R7)

        Return: Rd contains result, N,Z,C,V affected (including MOV).
        Execution Time: 1S*/
        bool I = (instr>>10)&0x1;//Immediate
        bool S = (instr>>9)&0x1;// Subtract
        uint32_t Op2;
        PC += 2;
        if(I){Op2 = (instr>>6)&0x7;}
        else{Op2 = registers[(instr>>6)&0x7];}
        uint8_t Rs = (instr>>3)&0x7;
        uint8_t Rd = (instr)&0x7;
        if(S){
          NZ_check(registers[Rs] - Op2);
          set_bit(&CPSR, 29, (registers[Rs] >= Op2));
          bool v_set = ((int32_t)registers[Rs]>0 && ((int32_t)Op2)<0 && N_FLAG) ||
                        ((int32_t)registers[Rs]<0 && ((int32_t)Op2)>0 && (!N_FLAG));
          set_bit(&CPSR, 28, v_set);
          registers[Rd] = registers[Rs] - Op2;
        }else{
          NZ_check(registers[Rs] + Op2);
          set_bit(&CPSR, 29, (uint64_t) registers[Rs] + (uint64_t) Op2 > UINT32_MAX);
          bool v_set = (registers[Rs]>0 && Op2>0 && N_FLAG) ||
                        (registers[Rs]<0 && Op2<0 && (!N_FLAG));
          set_bit(&CPSR, 28, v_set);
          registers[Rd] = registers[Rs] + Op2;
        }
        return 1; // 1S
      }
    }
    case 1:{// 3 - Immediate
      /*THUMB.3: move/compare/add/subtract immediate
        12-11  Opcode
                00b: MOV{S} Rd,#nn      ;move     Rd   = #nn
                01b: CMP    Rd,#nn      ;compare  Void = Rd - #nn
                10b: ADD{S} Rd,#nn      ;add      Rd   = Rd + #nn
                11b: SUB{S} Rd,#nn      ;subtract Rd   = Rd - #nn
      Return: Rd contains result (except CMP), N,Z,C,V affected (for MOV only N,Z).*/
      uint8_t Op = (instr>>11)&0x3;
      uint8_t Rd = (instr>>8)&0x7;
      uint8_t imm = (instr)&0xFF;
      switch (Op){
        case 0://MOV
          registers[Rd] = imm;
          NZ_check(imm);
          break;
        case 1://CMP
          NZ_check(registers[Rd] - imm);
          set_bit(&CPSR, 29, (registers[Rd] >= imm));
          bool v_set = ((int32_t)registers[Rd]>0 && ((int32_t)imm)<0 && N_FLAG) ||
                        ((int32_t)registers[Rd]<0 && ((int32_t)imm)>0 && (!N_FLAG));
          set_bit(&CPSR, 28, v_set);
          break;
        case 2://Add
          NZ_check(registers[Rd] + imm);
          set_bit(&CPSR, 29, (uint64_t) registers[Rd] + (uint64_t) imm > UINT32_MAX);
          v_set = (registers[Rd]>0 && imm>0 && N_FLAG) ||
                        (registers[Rd]<0 && imm<0 && (!N_FLAG));
          set_bit(&CPSR, 28, v_set);
          registers[Rd] += imm;
          break;
        case 3://SUB
          NZ_check(registers[Rd] - imm);
          set_bit(&CPSR, 29, (registers[Rd] >= imm));
          v_set = ((int32_t)registers[Rd]>0 && ((int32_t)imm)<0 && N_FLAG) ||
                        ((int32_t)registers[Rd]<0 && ((int32_t)imm)>0 && (!N_FLAG));
          set_bit(&CPSR, 28, v_set);
          registers[Rd] -= imm;
          break;
        default:
          printf("Thumb 3 IMM Op err\n");
          Failure(5);
          return -1;
        }
      time = 1; // 1S
      PC += 2; // next instruction
      break;
    }
    case 2:{// 4-8
      switch((instr & 0x1E00) >> 9){
        case 0x0:case 0x1:{// 4
          /*
          THUMB.4: ALU operations
            9-6    Opcode (0-Fh)
              0: AND{S} Rd,Rs     ;AND logical       Rd = Rd AND Rs
              1: EOR{S} Rd,Rs     ;XOR logical       Rd = Rd XOR Rs
              2: LSL{S} Rd,Rs     ;log. shift left   Rd = Rd << (Rs AND 0FFh)
              3: LSR{S} Rd,Rs     ;log. shift right  Rd = Rd >> (Rs AND 0FFh)
              4: ASR{S} Rd,Rs     ;arit shift right  Rd = Rd SAR (Rs AND 0FFh)
              5: ADC{S} Rd,Rs     ;add with carry    Rd = Rd + Rs + Cy
              6: SBC{S} Rd,Rs     ;sub with carry    Rd = Rd - Rs - NOT Cy
              7: ROR{S} Rd,Rs     ;rotate right      Rd = Rd ROR (Rs AND 0FFh)
              8: TST    Rd,Rs     ;test            Void = Rd AND Rs
              9: NEG{S} Rd,Rs     ;negate            Rd = 0 - Rs
              A: CMP    Rd,Rs     ;compare         Void = Rd - Rs
              B: CMN    Rd,Rs     ;neg.compare     Void = Rd + Rs
              C: ORR{S} Rd,Rs     ;OR logical        Rd = Rd OR Rs
              D: MUL{S} Rd,Rs     ;multiply          Rd = Rd * Rs
              E: BIC{S} Rd,Rs     ;bit clear         Rd = Rd AND NOT Rs
              F: MVN{S} Rd,Rs     ;not               Rd = NOT Rs

          ARM equivalent for NEG would be RSBS.
          Return: Rd contains result (except TST,CMP,CMN),
          Affected Flags:

          N,Z,C,V for  ADC,SBC,NEG,CMP,CMN
          N,Z,C   for  LSL,LSR,ASR,ROR (carry flag unchanged if zero shift amount)
          N,Z,C   for  MUL on ARMv4 and below: carry flag destroyed
          N,Z     for  MUL on ARMv5 and above: carry flag unchanged
          N,Z     for  AND,EOR,TST,ORR,BIC,MVN
          */
          uint8_t Rs = (instr>>3)&0x7;
          uint8_t Rd = (instr)&0x7;
          uint8_t Op = (instr>>6)&0xF;
          PC += 2;
          switch (Op)
          {
          case 0x0:{//AND
            registers[Rd] &= registers[Rs];
            NZ_check(registers[Rd]);
            break;
          }
          case 0x1:{//EOR
            registers[Rd] ^= registers[Rs];
            NZ_check(registers[Rd]);
            break;
          }
          case 0x2:{//LSL
            if(registers[Rs]&0xFF){
              set_bit(&CPSR, 29, (registers[Rd] << ((registers[Rs]&0xFF)-1))&0x80000000);
            }
            if((registers[Rs]&0xFF) >= 32){
              registers[Rd] = 0;
            }else{
              registers[Rd] <<= (registers[Rs]&0xFF);
            }
            NZ_check(registers[Rd]);
            break;
          }
          case 0x3:{//LSR
            if(registers[Rs]&0xFF){
              set_bit(&CPSR, 29, (registers[Rd] >> ((registers[Rs]&0xFF)-1))&0x1);
            }
            if((registers[Rs]&0xFF) >= 32){
              registers[Rd] = 0;
            }else{
              registers[Rd] >>= (registers[Rs]&0xFF);
            }
            NZ_check(registers[Rd]);
            break;
          }
          case 0x4:{//ASR
            if(registers[Rs]&0xFF){
              set_bit(&CPSR, 29, ((signed)registers[Rd] >> ((registers[Rs]&0xFF)-1))&0x1);
            }
            (registers[Rd]) = ((signed)registers[Rd]) >> (registers[Rs]&0xFF);
            NZ_check(registers[Rd]);
            break;
          }
          case 0x5:{//ADC
            bool hadC = C_FLAG;
            NZ_check(registers[Rd] + registers[Rs] + C_FLAG);
            set_bit(&CPSR, 29, (uint64_t) registers[Rd] + (uint64_t) registers[Rs] + C_FLAG > UINT32_MAX);
            bool v_set = (registers[Rd]>0 && registers[Rs]>0 && N_FLAG) ||
                        (registers[Rd]<0 && registers[Rs]<0 && (!N_FLAG));
            set_bit(&CPSR, 28, v_set);
            registers[Rd] += registers[Rs] + hadC;
            break;
          }
          case 0x6:{//SBC
            bool hadC = C_FLAG;
            NZ_check((registers[Rd] - registers[Rs]) - (!C_FLAG));
            set_bit(&CPSR, 29, (registers[Rd]-!C_FLAG >= registers[Rs]));
            bool v_set = ((int32_t)registers[Rd]>0 && (int32_t)registers[Rs]<0 && N_FLAG) ||
                        ((int32_t)registers[Rd]<0 && (int32_t)registers[Rs]>0 && (!N_FLAG));
            set_bit(&CPSR, 28, v_set);
            registers[Rd] -= registers[Rs] + !hadC;
            break;
          }
          case 0x7:{//ROR
            if(registers[Rs]&0xFF){
              set_bit(&CPSR, 29, ((signed)registers[Rd] >> ((registers[Rs]&0xFF)-1))&0x1);
            }
            registers[Rd] = ROR(registers[Rd], (registers[Rs]&0xFF));
            NZ_check(registers[Rd]);
            break;
          }
          case 0x8:{//TST
            NZ_check(registers[Rd] & registers[Rs]);
            break;
          }
          case 0x9:{//NEG
            NZ_check(-registers[Rs]);
            set_bit(&CPSR, 29, ((int64_t)(~(registers[Rs])) + 1)>UINT32_MAX); //?????
            bool v_set = (registers[Rs] == 0x80000000);
            set_bit(&CPSR, 28, v_set);
            registers[Rd] = -registers[Rs];
            break;
          }
          case 0xA:{//CMP
            NZ_check(registers[Rd] - registers[Rs]);
            set_bit(&CPSR, 29, (registers[Rd] >= registers[Rs]));
            bool v_set = ((int32_t)registers[Rd]>0 && ((int32_t)registers[Rs])<0 && N_FLAG) ||
                        ((int32_t)registers[Rd]<0 && ((int32_t)registers[Rs])>0 && (!N_FLAG));
            set_bit(&CPSR, 28, v_set);
            break;
          }
          case 0xB:{//CMN
            NZ_check(registers[Rd] + registers[Rs]);
            set_bit(&CPSR, 29, (uint64_t) registers[Rd] + (uint64_t) registers[Rs] > UINT32_MAX);
            bool v_set = (registers[Rd]>0 && registers[Rs]>0 && N_FLAG) ||
                        (registers[Rd]<0 && registers[Rs]<0 && (!N_FLAG));
            set_bit(&CPSR, 28, v_set);
            break;
          }
          case 0xC:{//ORR
            registers[Rd] |= registers[Rs];
            NZ_check(registers[Rd]);
            break;
          }
          case 0xD:{//MUL
            registers[Rd] *= registers[Rs];
            NZ_check(registers[Rd]);
            break;
          }
          case 0xE:{//BIC
            registers[Rd] &= ~registers[Rs];
            NZ_check(registers[Rd]);
            break;
          }
          case 0xF:{//MVN
            registers[Rd] = ~registers[Rs];
            NZ_check(registers[Rd]);
            break;
          }
          default:
            printf("THUMB 4 Op - %d\n", Op); Failure(5); return -1;
            break;
          }
          /*Execution Time:
            1S      for  AND,EOR,ADC,SBC,TST,NEG,CMP,CMN,ORR,BIC,MVN
            1S+1I   for  LSL,LSR,ASR,ROR
            1S+mI   for  MUL on ARMv4 (m=1..4; depending on MSBs of incoming Rd value)
            1S+mI   for  MUL on ARMv5 (m=3;  slow, no matter of MSBs of Rd value)*/
          return 1;
        }
        case 0x2:case 0x3:{// 5
          uint8_t Op = (instr>>8)&0x3;
          uint8_t Rs = (instr>>3)&0xF;
          uint8_t Rd = ((instr)&0x7) | ((instr>>4)&0x8);
          /*
          THUMB.5: Hi register operations/branch exchange
            9-8    Opcode (0-3)
                    0: ADD Rd,Rs   ;add        Rd = Rd+Rs
                    1: CMP Rd,Rs   ;compare  Void = Rd-Rs  ;CPSR affected
                    2: MOV Rd,Rs   ;move       Rd = Rs
                    3: BX  Rs      ;jump       PC = Rs     ;may switch THUMB/ARM
                    3: BLX Rs      ;call       PC = Rs     ;may switch THUMB/ARM (ARM9)

          Restrictions: For ADD/CMP/MOV, MSBs and/or MSBd must be set, ie. it is not allowed that both are cleared.
          When using R15 (PC) as operand, the value will be the address of the instruction plus 4 (ie. $+4). Except for BX R15: CPU switches to ARM state, and PC is auto-aligned as (($+4) AND NOT 2).
          For BX, MSBs may be 0 or 1, MSBd must be zero, Rd is not used/zero.
          For BLX, MSBs may be 0 or 1, MSBd must be set, Rd is not used/zero.
          For BX/BLX, when Bit 0 of the value in Rs is zero:

            Processor will be switched into ARM mode!
            If so, Bit 1 of Rs must be cleared (32bit word aligned).
            Thus, BX PC (switch to ARM) may be issued from word-aligned address
            only, the destination is PC+4 (ie. the following halfword is skipped).

          BLX may not use R15. BLX saves the return address as LR=PC+3 (with thumb bit).
          Using BLX R14 is possible (sets PC=Old_LR, and New_LR=retadr).
          Assemblers/Disassemblers should use MOV R8,R8 as NOP (in THUMB mode).

          Return: Only CMP affects CPSR condition flags!
          Execution Time:
          1S     for ADD/MOV/CMP
          2S+1N  for ADD/MOV with Rd=R15, and for BX
          */
          
          uint32_t data;
          PC += 2;
          if(Rs == 15){
            data = PC+4;
            time = 2; // 2S + 1N
          }else{
            data = registers[Rs];
          }
          switch (Op){
            case 0x0:// ADD
              time = 1;
              if(Rd == 15){PC+=2;}
              registers[Rd] += data;
              break;
            case 0x1:// CMP
              time = 1;
              NZ_check(registers[Rd] - data);
              break;
            case 0x2:// MOV
              time = 1;
              registers[Rd] = data;
              break;
            case 0x3://Branch
              bool L = (instr>>7)&0x1;
              if(L){
                LR = PC+1;
              }
              PC = data;
              if(!(data&0x1)){
                // Arm mode time!
                THUMB = false;
                if(PC%4){PC -= (PC%4);}
              }
              if(PC%2){PC--;}
              return 2; // 2S + 1N
            default:
              printf("THUMB 5 Op err - 0x%.1X\n", Op);
              Failure(5);
              return -1;
          }
          if(PC%2){PC--;}
          break;
        }
        case 0x4:case 0x5:case 0x6:case 0x7:{// 6
          uint8_t Rd = (instr>>8)&0x7;
          uint32_t nn = (instr&0xFF) * 4;
          /*
          THUMB.6: load PC-relative (for loading immediates from literal pool)
                    LDR Rd,[PC,#nn]      ;load 32bit    Rd = WORD[PC+nn]
            10-8   Rd - Destination Register   (R0..R7)
            7-0    nn - Unsigned offset        (0-1020 in steps of 4)
          The value of PC will be interpreted as (($+4) AND NOT 2).
          Return: No flags affected, data loaded into Rd.
          */
          uint32_t addr = ((PC+4) + nn);
          registers[Rd] = gba_read32( addr - (addr%4));
          PC += 2;
          return 1; //1S+1N+1I
        }
        case 0x8:case 0xA:case 0xC:case 0xE:{// 7
          /*
          11-10  Opcode (0-3)
                  0: STR  Rd,[Rb,Ro]   ;store 32bit data  WORD[Rb+Ro] = Rd
                  1: STRB Rd,[Rb,Ro]   ;store  8bit data  BYTE[Rb+Ro] = Rd
                  2: LDR  Rd,[Rb,Ro]   ;load  32bit data  Rd = WORD[Rb+Ro]
                  3: LDRB Rd,[Rb,Ro]   ;load   8bit data  Rd = BYTE[Rb+Ro]
          8-6    Ro - Offset Register              (R0..R7)
          5-3    Rb - Base Register                (R0..R7)
          2-0    Rd - Source/Destination Register  (R0..R7)
          Return: No flags affected, data loaded either into Rd or into memory.
          */
          uint8_t Op = (instr>>10)&0x3;
          uint8_t Ro = (instr>>6)&0x7;
          uint8_t Rb = (instr>>3)&0x7;
          uint8_t Rd = (instr)&0x7;
          uint32_t addr = registers[Rb] + registers[Ro];
          PC+=2;
          switch (Op){
            case 0://STR
              gba_write32(addr, registers[Rd]);
            break;
            case 1://STR byte
              gba_write8(addr, registers[Rd]);
            break;
            case 2://LD
              registers[Rd] = gba_read32(addr);
              if(addr%4)
                registers[Rd] = ROR(registers[Rd], ((addr)%4)*8);
            break;
            case 3://LD byte
              registers[Rd] = gba_read8(addr);
              if(addr%4)
                registers[Rd] = ROR(registers[Rd], ((addr)%4)*8);
            break;
          }
          PC-=PC%2;
          return 1; //1S+1N+1I for LDR, or 2N for STR
        }
        case 0x9:case 0xB:case 0xD:case 0xF:{// 8
          /*
          THUMB.8: load/store sign-extended byte/halfword
            11-10  Opcode (0-3)
                    0: STRH Rd,[Rb,Ro]  ;store 16bit data          HALFWORD[Rb+Ro] = Rd
                    1: LDSB Rd,[Rb,Ro]  ;load sign-extended 8bit   Rd = BYTE[Rb+Ro]
                    2: LDRH Rd,[Rb,Ro]  ;load zero-extended 16bit  Rd = HALFWORD[Rb+Ro]
                    3: LDSH Rd,[Rb,Ro]  ;load sign-extended 16bit  Rd = HALFWORD[Rb+Ro]
            8-6    Ro - Offset Register              (R0..R7)
            5-3    Rb - Base Register                (R0..R7)
            2-0    Rd - Source/Destination Register  (R0..R7)
          Execution Time: 1S+1N+1I for LDR, or 2N for STR*/
          uint8_t Op = (instr>>10)&0x3;
          uint8_t Ro = (instr>>6)&0x7;
          uint8_t Rb = (instr>>3)&0x7;
          uint8_t Rd = (instr)&0x7;
          PC+=2;
          uint32_t addr = registers[Rb] + registers[Ro];
          switch (Op){
            case 0://STR h/word
              gba_write16(addr, registers[Rd]);
            break;
            case 1://LD byte
              registers[Rd] = sign_extend(gba_read8(addr), 8);
            break;
            case 2://LD
              registers[Rd] = (uint32_t)gba_read16(addr);
              if(addr%4)
                registers[Rd] = ROR(registers[Rd], ((addr)%4)*8);
            break;
            case 3://LD h/word
              registers[Rd] = sign_extend(gba_read16(addr),16);
              if(addr%4)
                registers[Rd] = sign_extend(ROR(registers[Rd], ((addr)%4)*8),16);
            break;
          }
          PC-=PC%2;
          return 1; //1S+1N+1I for LDR, or 2N for STR
        }
      }
      break;
    }
    case 3:{// 9 - LD/ST w/ Immediate
      /*
        12-11  Opcode (0-3)
                0: STR  Rd,[Rb,#nn]  ;store 32bit data   WORD[Rb+nn] = Rd
                1: LDR  Rd,[Rb,#nn]  ;load  32bit data   Rd = WORD[Rb+nn]
                2: STRB Rd,[Rb,#nn]  ;store  8bit data   BYTE[Rb+nn] = Rd
                3: LDRB Rd,[Rb,#nn]  ;load   8bit data   Rd = BYTE[Rb+nn]
        10-6   nn - Unsigned Offset              (0-31 for BYTE, 0-124 for WORD)
        5-3    Rb - Base Register                (R0..R7)
        2-0    Rd - Source/Destination Register  (R0..R7)

      Return: No flags affected, data loaded either into Rd or into memory.
      Execution Time: 1S+1N+1I for LDR, or 2N for STR*/
      uint8_t Op = (instr>>11)&0x3;
      uint8_t off = (instr>>6)&0x1F;
      uint8_t Rb = (instr>>3)&0x7;
      uint8_t Rd = (instr)&0x7;
      PC+=2;
      switch (Op){
        case 0://STR
          gba_write32(registers[Rb]+(off*4), registers[Rd]);
        break;
        case 1://LD
          registers[Rd] = gba_read32(registers[Rb]+(off*4));
          if(registers[Rb]+(off)%4)
            registers[Rd] = ROR(registers[Rd], ((registers[Rb]+(off*4))%4)*8);
        break;
        case 2://STR byte
          gba_write8(registers[Rb]+(off), registers[Rd]);
        break;
        case 3://LD byte
          registers[Rd] = gba_read8(registers[Rb]+(off));
          if(registers[Rb]+(off)%4)
            registers[Rd] = ROR(registers[Rd], ((registers[Rb]+(off))%4)*8);
        break;
      }
      PC-=PC%2;
      return 1; //1S+1N+1I for LDR, or 2N for STR
    }
    case 4:{// 10-11
      if(!((instr>>12)&0x1)){// 10 - l/s Half word
        /*
        THUMB.10: load/store halfword
          11     Opcode (0-1)
                  0: STRH Rd,[Rb,#nn]  ;store 16bit data   HALFWORD[Rb+nn] = Rd
                  1: LDRH Rd,[Rb,#nn]  ;load  16bit data   Rd = HALFWORD[Rb+nn]
          10-6   nn - Unsigned Offset              (0-62, step 2)
          5-3    Rb - Base Register                (R0..R7)
          2-0    Rd - Source/Destination Register  (R0..R7)
        Return: No flags affected, data loaded either into Rd or into memory.
        Execution Time: 1S+1N+1I for LDR, or 2N for STR*/
        uint8_t nn = ((instr>>6)&0x1F)*2;
        uint8_t Rb = (instr>>3)&0x7;
        uint8_t Rd = (instr)&0x7;
        if((instr>>11)&0x1){// Load
          registers[Rd] = gba_read16(registers[Rb]+nn);
          if(registers[Rb]+(nn)%4)
            registers[Rd] = ROR(registers[Rd], ((registers[Rb]+(nn))%4)*8);
        }else{// Store
          gba_write16(registers[Rb]+nn, registers[Rd]);
        }
        PC+=2;
        return 1; //1S+1N+1I for LDR, or 2N for STR
      }else{// 11 - l/s SP
        /*
        THUMB.11: load/store SP-relative
          11     Opcode (0-1)
                  0: STR  Rd,[SP,#nn]  ;store 32bit data   WORD[SP+nn] = Rd
                  1: LDR  Rd,[SP,#nn]  ;load  32bit data   Rd = WORD[SP+nn]
          10-8   Rd - Source/Destination Register  (R0..R7)
          7-0    nn - Unsigned Offset              (0-1020, step 4)
        Execution Time: 1S+1N+1I for LDR, or 2N for STR*/
        uint8_t Rd = (instr>>8)&0x7;
        uint8_t nn = ((instr)&0xFF)*4;
        if((instr>>11)&0x1){// Load
          registers[Rd] = gba_read32(SP+nn);
          if(SP+(nn)%4)
            registers[Rd] = ROR(registers[Rd], ((SP+(nn))%4)*8);
        }else{// Store
          gba_write32(SP+nn, registers[Rd]);
        }
        PC+=2;
        return 1; //1S+1N+1I for LDR, or 2N for STR
      }
    }
    case 5:{// 12-14
      if(!((instr>>12)&0x1)){// 12 - Add PC/SP
        uint8_t Rd = (instr>>8)&0x7;
        uint16_t offset = ((instr)&0xFF)*4;
        PC += 2;
        if(((instr>>11)&0x1)){
          // SP
          registers[Rd] = SP + offset;
        }else{
          // PC
          if(offset == 4){offset = 2;}
          registers[Rd] = ((PC+2)) + offset;
        }
        time = 1; // 1S
      }else{
        if(!((instr>>10)&0x1)){// 13 - Add SP+nn
          /*
          THUMB.13: add offset to stack pointer
            15-8   Must be 10110000b for this type of instructions
            7      Opcode/Sign
                    0: ADD  SP,#nn       ;SP = SP + nn
                    1: SUB  SP,#nn       ;SP = SP - nn
            6-0    nn - Unsigned Offset    (0-508, step 4)
          Return: No flags affected, SP adjusted.
          Execution Time: 1S*/
          uint32_t offset = ((instr)&0x7F)*4;
          if(((instr>>7)&0x1)){
            SP -= offset;
          }else{
            SP += offset;
          }
          PC += 2;
          return 1; //1S
        }else{// 14 - Push-Pop
          /*
          THUMB.14: push/pop registers
            11     Opcode (0-1)
                    0: PUSH {Rlist}{LR}   ;store in memory, decrements SP (R13)
                    1: POP  {Rlist}{PC}   ;load from memory, increments SP (R13)
            8      PC/LR Bit (0-1)
                    0: No
                    1: PUSH LR (R14), or POP PC (R15)
            7-0    Rlist - List of Registers (R7..R0)
          Note: When calling to a sub-routine, the return address is stored in LR register, when calling further sub-routines, PUSH {LR} must be used to save higher return address on stack. If so, POP {PC} can be later used to return from the sub-routine.
          POP {PC} ignores the least significant bit of the return address (processor remains in thumb state even if bit0 was cleared), when intending to return with optional mode switch, use a POP/BX combination (eg. POP {R3} / BX R3).
          ARM9: POP {PC} copies the LSB to thumb bit (switches to ARM if bit0=0).
          Execution Time: nS+1N+1I (POP), (n+1)S+2N+1I (POP PC), or (n-1)S+2N (PUSH).*/
          uint8_t rlist= instr&0xFF;
          bool PC_LR_BIT = (instr>>8)&0x1;
          bool POP = (instr>>11)&0x1;
          int moved = 0;
          PC += 2;
          if(POP){
            for(int x=7; x>=0; x--){
              if((rlist>>x)&0x1){
                SP+=4;
                registers[x] = gba_read32(SP);
                moved++;
              }
            }
            if(PC_LR_BIT){
              SP+=4;
              PC = gba_read32(SP);
              PC -= PC%2;
              moved += 2;
              return moved; //(n+1)S+2N+1I (POP PC)
            }
            return moved; //nS+1N+1I (POP)
          }else{//PUSH
            if(PC_LR_BIT){
              gba_write32(SP, LR);
              SP-=4;
              moved++;
            }
            for(int x=0; x<8; x++){
              if((rlist>>x)&0x1){
                gba_write32(SP, registers[x]);
                SP-=4;
                moved++;
              }
            }
            return moved - 1; //(n-1)S+2N (PUSH)
          }
        }
      }
      break;
    }
    case 6:{// 15-17
      if(!((instr>>12)&0x1)){// 15 - STM/LDM
        /*
        THUMB.15: multiple load/store
          11     Opcode (0-1)
                  0: STMIA Rb!,{Rlist}   ;store in memory, increments Rb
                  1: LDMIA Rb!,{Rlist}   ;load from memory, increments Rb
          10-8   Rb - Base register (modified) (R0-R7)
          7-0    Rlist - List of Registers     (R7..R0)
        Both STM and LDM are incrementing the Base Register.
        The lowest register in the list (ie. R0, if it's in the list) is stored/loaded at the lowest memory address.

        Strange Effects on Invalid Rlist's
        Empty Rlist: R15 loaded/stored (ARMv4 only), and Rb=Rb+40h (ARMv4-v5).
        Writeback with Rb included in Rlist: Store OLD base if Rb is FIRST entry in Rlist, otherwise store NEW base (STM/ARMv4), always store OLD base (STM/ARMv5), no writeback (LDM/ARMv4/ARMv5; at this point, THUMB opcodes work different than ARM opcodes).*/
        uint8_t rlist= instr&0xFF;
        bool L = (instr>>11)&0x1;
        uint8_t Rb = (instr>>8)&0x7;
        int moved = 0;
        PC += 2;
        if(rlist == 0){
          if(L){
            PC = gba_read32(registers[Rb]);
          }else{
            gba_write32(registers[Rb], PC+6);
          }
          registers[Rb]+=0x40;
          return 1;
        }
        if(L){//Load
          for(int x=0; x<8; x++){
            if((rlist>>x)&0x1){
              registers[x] = gba_read32(registers[Rb]);
              registers[Rb]+=4;
              moved++;
            }
          }
          return moved; //nS+1N+1I (LD)
        }else{//Store
          bool first = true;
          uint32_t base = 0;
          for(int x=0; x<8; x++){
            if((rlist>>x)&0x1){
              if(Rb == x){
                if(!first){
                  base = registers[Rb];
                }
              }
              gba_write32(registers[Rb], registers[x]);
              registers[Rb]+=4;
              moved++;
              first = false;
            }
          }
          if(base){
            gba_write32(base, registers[Rb]);
          }
          return moved - 1; //(n-1)S+2N (STORE)
        }
      }else{// 16-17
        uint8_t Cond = (instr>>8)&0xF;
        if(Cond == 0xE){// U?
          printf("THUMB U\n"); Failure(5); return -1;
        }else if(Cond == 0xF){// 17 - SWI
          PC += 2;
          return software_int(instr & 0xFF);
        }else{// 16 - Branch
          /*THUMB.16: conditional branch
            11-8   Opcode/Condition (0-Fh)
            7-0    Signed Offset, step 2 ($+4-256..$+4+254)
          */
          if(condition(Cond)){
            int16_t offset = (int8_t)(instr&0xFF);
            offset *= 2; 
            PC += 4 + offset;
            PC -= PC%2;
            return 2; // 2S+1N
          }else{
            PC += 2;
            return 1; // 1S
          }
        }
      }
    }
    case 7:{// 18-19
      uint8_t mode = (instr>>11)&0x3;
      if(mode == 0){// 18 - B
        /*
        THUMB.18: unconditional branch
          10-0   Signed Offset, step 2 ($+4-2048..$+4+2046)
        */
        int16_t offset = (int16_t)(instr&0x7FF);
        offset |= (instr&0x400)?0xF800:0;//sign extend
        offset *= 2;
        PC += 4 + offset;
        return 2; // 2S+1N
      }else{// 19 - BLX
        /*THUMB.19: long branch with link
        This may be used to call (or jump) to a subroutine, return address is saved in LR (R14).
        Unlike all other THUMB mode instructions, this instruction occupies 32bit of memory which are split into two 16bit THUMB opcodes.

        First Instruction - LR = PC+4+(nn SHL 12)
          15-11  Must be 11110b for BL/BLX type of instructions
          10-0   nn - Upper 11 bits of Target Address
        Second Instruction - PC = LR + (nn SHL 1), and LR = PC+2 OR 1 (and BLX: T=0)
          15-11  Opcode
                  11111b: BL label   ;branch long with link
                  11101b: BLX label  ;branch long with link switch to ARM mode (ARM9)
          10-0   nn - Lower 11 bits of Target Address (BLX: Bit0 Must be zero)

        The destination address range is (PC+4)-400000h..+3FFFFEh, ie. PC+/-4M.
        Target must be halfword-aligned. As Bit 0 in LR is set, it may be used to return by a BX LR instruction (keeping CPU in THUMB mode).
        Return: No flags affected, PC adjusted, return address in LR.
        Execution Time: 3S+1N (first opcode 1S, second opcode 2S+1N).
        Note: Exceptions may or may not occur between first and second opcode, this is "implementation defined" (unknown how this is implemented in GBA and NDS).
        Using only the 2nd half of BL as "BL LR+imm" is possible (for example, Mario Golf Advance Tour for GBA uses opcode F800h as "BL LR+0").*/
        uint16_t instr2 = gba_read16(PC+2);
        int32_t offset = sign_extend(((instr&0x7FF)<<12) | ((instr2&0x7FF)<<1), 23);
        LR = (PC+4) | 1;
        PC += 4+offset;
        if(!((instr2>>12)&0x1)){
          THUMB = false;
          if(PC%4){PC -= (PC%4);}
        }
        if(PC%2){PC--;}
        return 3; //3S + 1N
      }
    }
    default:
      Failure(5);
      return -1;
    }
    return time;
}


// ================== Debug Functions ==============

void gbaCPU_print_cycle(){
  printf("%.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X %.8X cpsr: %.8X \n", 
    registers[0],registers[1],registers[2],registers[3],registers[4],registers[5],registers[6],registers[7],
    registers[8],registers[9],registers[10],registers[11],registers[12],registers[13],registers[14],registers[15]+4,
    CPSR);
}

inline uint32_t get_reg(int r){
  return registers[r];
}

inline uint32_t get_cpsr(){
  return CPSR;
}