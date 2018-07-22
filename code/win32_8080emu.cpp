/*
Project: Intel 8080 CPU Emulator
File: win32_8080emu.cpp
Author: Brock Salmon
Notice: (C) Copyright 2018 by Brock Salmon. All Rights Reserved.
*/

/*
NOTE(bSalmon):

8080_INTERNAL:
0 - Public Build
1 - Dev Build

8080_SLOW:
 0 - Debugging Code Disabled
 1 - Non-performant debugging code enabled
*/

/*
MEMORY LAYOUT:
0000 - 1fff: ROM Memory
2000 - 23ff: Work RAM
2400 - 3fff: Video RAM
*/

#include <Windows.h>
#include <stdio.h>
#include "8080emu.h"

#if EMU8080_INTERNAL
//#include "8080emu_disassemble.cpp"
#endif

inline u8 SetParityFlag(u8 x, u8 size)
{
	u8 parityCheckByte = 0;
	for (int i = 0; i < size; i++)
	{
		if ((x & (1 << i)) == (1 << i))
		{
			parityCheckByte++;
		}
	}
	
	return (0 == (parityCheckByte & 0x01));
}

inline u8 SetAuxiliaryFlag(u8 a, u8 b, b32 isAddition)
{
	a <<= 4;
	a >>= 4;
	b <<= 4;
	b >>= 4;
	
	if (isAddition)
	{
		return ((a + b) >= 0x10);
	}
	else
	{
		return ((a - b) > a);
	}
}

internal_func void Emulate(CPUState *cpuState, unsigned char *castedMem)
{
	u8 *opCode = &castedMem[cpuState->programCounter];
	
	u8 opBytes = 1;
	
	switch(*opCode)
	{
		// 0x0 ///////////////////////////////////////////////////////////////////////////
		
		case 0x01:
		{
			// LXI B,D16
			opBytes = 3;
			cpuState->regB = opCode[2];
			cpuState->regC = opCode[1];
			break;
		}
		
		case 0x02:
		{
			// STAX B
			u16 pairBC = (cpuState->regB << 8) | cpuState->regC;
			cpuState->memory[pairBC] = cpuState->regA;
			break;
		}
		
		case 0x03:
		{
			// INX B
			cpuState->regC++;
			if (cpuState->regC == 0x00)
			{
				cpuState->regB++;
			}
			break;
		}
		
		case 0x04:
		{
			// INR B
			u8 result = cpuState->regB + 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regB, 1, true);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regB = result;
			break;
		}
		
		case 0x05:
		{
			// DCR B
			u8 result = cpuState->regB - 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regB, 1, false);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regB = result;
			break;
		}
		
		case 0x06:
		{
			// MVI B,a8
			opBytes = 2;
			cpuState->regB = opCode[1];
			break;
		}
		
		case 0x07:
		{
			// RLC
			cpuState->regF.c = (cpuState->regA & (1<<7)) == (1<<7);
			cpuState->regA = cpuState->regA << 1;
			if (cpuState->regF.c)
			{
				cpuState->regA = cpuState->regA | 0x01;
			}
			break;
		}
		
		case 0x09:
		{
			// DAD B
			u16 pairBC = (cpuState->regB << 8) | cpuState->regC;
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = pairHL + pairBC;
			cpuState->regF.c = result < pairHL;
			pairHL = result;
			break;
		}
		
		case 0x0a:
		{
			// LDAX B
			u16 pairBC = (cpuState->regB << 8) | cpuState->regC;
			cpuState->regA = cpuState->memory[pairBC];
			break;
		}
		
		case 0x0b:
		{
			// DCX B
			cpuState->regC--;
			if (cpuState->regC == 0xFF)
			{
				cpuState->regB--;
			}
			break;
		}
		
		case 0x0c:
		{
			// INR C
			u8 result = cpuState->regC + 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regC, 1, true);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regC = result;
			break;
		}
		
		case 0x0d:
		{
			// DCR C
			u8 result = cpuState->regC - 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regC, 1, false);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regC = result;
			break;
		}
		
		case 0x0e:
		{
			// MVI C,a8
			opBytes = 2;
			cpuState->regC = opCode[1];
			break;
		}
		
		case 0x0f:
		{
			// RRC
			cpuState->regF.c = (cpuState->regA & 0x1) == 0x01;
			cpuState->regA = cpuState->regA >> 1;
			if (cpuState->regF.c)
			{
				cpuState->regA = cpuState->regA | (1<<7);
			}
			break;
		}
		
		// 0x1 ///////////////////////////////////////////////////////////////////////////
		
		case 0x11:
		{
			// LXI D,D16
			opBytes = 3;
			cpuState->regD = opCode[2];
			cpuState->regE = opCode[1];
			break;
		}
		
		case 0x12:
		{
			// STAX D
			u16 pairDE = (cpuState->regD << 8) | cpuState->regE;
			cpuState->memory[pairDE] = cpuState->regA;
			break;
		}
		
		case 0x13:
		{
			// INX D
			cpuState->regE++;
			if (cpuState->regE == 0x00)
			{
				cpuState->regD++;
			}
			break;
		}
		
		case 0x14:
		{
			// INR D
			u8 result = cpuState->regD + 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regD, 1, true);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regD = result;
			break;
		}
		
		case 0x15:
		{
			// DCR D
			u8 result = cpuState->regD - 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regD, 1, false);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regD = result;
			break;
		}
		
		case 0x16:
		{
			// MVI D,a8
			opBytes = 2;
			cpuState->regD = opCode[1];
			break;
		}
		
		case 0x17:
		{
			// RAL
			u8 tempA = cpuState->regA << 1;
			tempA = tempA | cpuState->regF.c;
			
			cpuState->regF.c = (cpuState->regA & (1<<7)) == (1<<7);
			cpuState->regA = tempA;
			break;
		}
		
		case 0x19:
		{
			// DAD D
			u16 pairDE = (cpuState->regD << 8) | cpuState->regE;
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = pairHL + pairDE;
			cpuState->regF.c = result < pairHL;
			pairHL = result;
			break;
		}
		
		case 0x1a:
		{
			// LDAX D
			u16 pairDE = (cpuState->regD << 8) | cpuState->regE;
			cpuState->regA = cpuState->memory[pairDE];
			break;
		}
		
		case 0x1b:
		{
			// DCX D
			cpuState->regE--;
			if (cpuState->regE == 0xFF)
			{
				cpuState->regD--;
			}
			break;
		}
		
		case 0x1c:
		{
			// INR E
			u8 result = cpuState->regE + 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regE, 1, true);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regE = result;
			break;
		}
		
		case 0x1d:
		{
			// DCR E
			u8 result = cpuState->regE - 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regE, 1, false);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regE = result;
			break;
		}
		
		case 0x1e:
		{
			// MVI E,a8
			opBytes = 2;
			cpuState->regE = opCode[1];
			break;
		}
		
		case 0x1f:
		{
			// RAR
			u8 tempA = cpuState->regA >> 1;
			tempA = tempA | (cpuState->regF.c << 7);
			
			cpuState->regF.c = (cpuState->regA & 0x1) == 0x01;
			cpuState->regA = tempA;
			break;
		}
		
		// 0x2 ///////////////////////////////////////////////////////////////////////////
		
		case 0x21:
		{
			// LXI H,D16
			opBytes = 3;
			cpuState->regH = opCode[2];
			cpuState->regL = opCode[1];
			break;
		}
		
		case 0x22:
		{
			// SHLD a16
			opBytes = 3;
			u16 adr = (opCode[2] << 8) | opCode[1];
			cpuState->memory[adr] = cpuState->regL;
			adr++;
			cpuState->memory[adr] = cpuState->regH;
			break;
		}
		
		case 0x23:
		{
			// INX H
			cpuState->regL++;
			if (cpuState->regL == 0x00)
			{
				cpuState->regH++;
			}
			break;
		}
		
		case 0x24:
		{
			// INR H
			u8 result = cpuState->regH + 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regH, 1, true);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regH = result;
			break;
		}
		
		case 0x25:
		{
			// DCR H
			u8 result = cpuState->regH - 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regH, 1, false);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regH = result;
			break;
		}
		
		case 0x26:
		{
			// MVI H,a8
			opBytes = 2;
			cpuState->regH = opCode[1];
			break;
		}
		
		case 0x27:
		{
			// DAA
			u8 lowBits = cpuState->regA;
			lowBits <<= 4;
			lowBits >>= 4;
			if (cpuState->regF.a || (lowBits > 0x09))
			{
				
			}
			
			u8 highBits = cpuState->regA;
			highBits >>= 4;
			if (cpuState->regF.c || (highBits > 0x09))
			{
				
			}
			
			ASSERT(false);
			break;
		}
		
		case 0x29:
		{
			// DAD H
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = pairHL + pairHL;
			cpuState->regF.c = result < pairHL;
			pairHL = result;
			break;
		}
		
		case 0x2a:
		{
			// LHLD a16
			opBytes = 3;
			u16 adr = (opCode[2] << 8) | opCode[1];
			cpuState->regL = cpuState->memory[adr];
			adr++;
			cpuState->regH = cpuState->memory[adr];
			break;
		}
		
		case 0x2b:
		{
			// DCX H
			cpuState->regL--;
			if (cpuState->regL == 0xFF)
			{
				cpuState->regH--;
			}
			break;
		}
		
		case 0x2c:
		{
			// INR L
			u8 result = cpuState->regL + 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regL, 1, true);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regL = result;
			break;
		}
		
		case 0x2d:
		{
			// DCR L
			u8 result = cpuState->regL - 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regL, 1, false);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regL = result;
			break;
		}
		
		case 0x2e:
		{
			// MVI L,a8
			opBytes = 2;
			cpuState->regL = opCode[1];
			break;
		}
		
		case 0x2f:
		{
			ASSERT(false);
			break;
		}
		
		// 0x3 ///////////////////////////////////////////////////////////////////////////
		
		case 0x31:
		{
			// LXI SP,D16
			opBytes = 3;
			cpuState->stackPointer = (opCode[2] << 8) | opCode[1];
			break;
		}
		
		case 0x32:
		{
			// STA a16
			opBytes = 3;
			u16 adr = (opCode[2] << 8) | opCode[1];
			cpuState->memory[adr] = cpuState->regA;
			break;
		}
		
		case 0x33:
		{
			// INX SP
			cpuState->stackPointer++;
			break;
		}
		
		case 0x34:
		{
			// INR M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u8 result = cpuState->memory[pairHL] + 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->memory[pairHL], 1, true);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->memory[pairHL] = result;
			break;
		}
		
		case 0x35:
		{
			// DCR M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u8 result = cpuState->memory[pairHL] - 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->memory[pairHL], 1, false);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->memory[pairHL] = result;
			break;
		}
		
		case 0x36:
		{
			// MVI M,a8
			opBytes = 2;
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->memory[pairHL] = opCode[1];
			break;
		}
		
		case 0x37:
		{
			ASSERT(false);
			break;
		}
		
		case 0x39:
		{
			// DAD SP
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = pairHL + cpuState->stackPointer;
			cpuState->regF.c = result < pairHL;
			pairHL = result;
			break;
		}
		
		case 0x3a:
		{
			// LDA a16
			opBytes = 3;
			u16 adr = (opCode[2] << 8) | opCode[1];
			cpuState->regA = cpuState->memory[adr];
			break;
		}
		
		case 0x3b:
		{
			// DCX SP
			cpuState->stackPointer--;
			break;
		}
		
		case 0x3c:
		{
			// INR A
			u8 result = cpuState->regA + 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, 1, true);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regA = result;
			break;
		}
		
		case 0x3d:
		{
			// DCR A
			u8 result = cpuState->regA - 1;
			cpuState->regF.z = (result == 0);
			cpuState->regF.s = (result & (1<<7)) == (1<<7);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, 1, false);
			cpuState->regF.p = SetParityFlag(result, 8);
			cpuState->regA = result;
			break;
		}
		
		case 0x3e:
		{
			// MVI A,a8
			opBytes = 2;
			cpuState->regA = opCode[1];
			break;
		}
		
		case 0x3f:
		{
			ASSERT(false);
			break;
		}
		
		// 0x4 ///////////////////////////////////////////////////////////////////////////
		
		case 0x40:
		{
			// MOV B,B
			cpuState->regB = cpuState->regB;
			break;
		}
		
		case 0x41:
		{
			// MOV B,C
			cpuState->regB = cpuState->regC;
			break;
		}
		
		case 0x42:
		{
			// MOV B,D
			cpuState->regB = cpuState->regD;
			break;
		}
		
		case 0x43:
		{
			// MOV B,E
			cpuState->regB = cpuState->regE;
			break;
		}
		
		case 0x44:
		{
			// MOV B,H
			cpuState->regB = cpuState->regH;
			break;
		}
		
		case 0x45:
		{
			// MOV B,L
			cpuState->regB = cpuState->regL;
			break;
		}
		
		case 0x46:
		{
			// MOV B,M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->regB = cpuState->memory[pairHL];
			break;
		}
		
		case 0x47:
		{
			// MOV B,A
			cpuState->regB = cpuState->regA;
			break;
		}
		
		case 0x48:
		{
			// MOV C,B
			cpuState->regC = cpuState->regB;
			break;
		}
		
		case 0x49:
		{
			// MOV C,C
			cpuState->regC = cpuState->regC;
			break;
		}
		
		case 0x4a:
		{
			// MOV C,D
			cpuState->regC = cpuState->regD;
			break;
		}
		
		case 0x4b:
		{
			// MOV C,E
			cpuState->regC = cpuState->regE;
			break;
		}
		
		case 0x4c:
		{
			// MOV C,H
			cpuState->regC = cpuState->regH;
			break;
		}
		
		case 0x4d:
		{
			// MOV C,L
			cpuState->regC = cpuState->regL;
			break;
		}
		
		case 0x4e:
		{
			// MOV C,M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->regC = cpuState->memory[pairHL];
			break;
		}
		
		case 0x4f:
		{
			// MOV C,A
			cpuState->regC = cpuState->regA;
			break;
		}
		
		// 0x5 ///////////////////////////////////////////////////////////////////////////
		
		case 0x50:
		{
			// MOV D,B
			cpuState->regD = cpuState->regB;
			break;
		}
		
		case 0x51:
		{
			// MOV D,C
			cpuState->regD = cpuState->regC;
			break;
		}
		
		case 0x52:
		{
			// MOV D,D
			cpuState->regD = cpuState->regD;
			break;
		}
		
		case 0x53:
		{
			// MOV D,E
			cpuState->regD = cpuState->regE;
			break;
		}
		
		case 0x54:
		{
			// MOV D,H
			cpuState->regD = cpuState->regH;
			break;
		}
		
		case 0x55:
		{
			// MOV D,L
			cpuState->regD = cpuState->regL;
			ASSERT(false);
			break;
		}
		
		case 0x56:
		{
			// MOV D,M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->regD = cpuState->memory[pairHL];
			break;
		}
		
		case 0x57:
		{
			// MOV D,A
			cpuState->regD = cpuState->regA;
			break;
		}
		
		case 0x58:
		{
			// MOV E,B
			cpuState->regE = cpuState->regB;
			break;
		}
		
		case 0x59:
		{
			// MOV E,C
			cpuState->regE = cpuState->regC;
			break;
		}
		
		case 0x5a:
		{
			// MOV E,D
			cpuState->regE = cpuState->regD;
			break;
		}
		
		case 0x5b:
		{
			// MOV E,E
			cpuState->regE = cpuState->regE;
			break;
		}
		
		case 0x5c:
		{
			// MOV E,H
			cpuState->regE = cpuState->regH;
			break;
		}
		
		case 0x5d:
		{
			// MOV E,L
			cpuState->regE = cpuState->regL;
			break;
		}
		
		case 0x5e:
		{
			// MOV E,M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->regE = cpuState->memory[pairHL];
			break;
		}
		
		case 0x5f:
		{
			// MOV E,A
			cpuState->regE = cpuState->regA;
			break;
		}
		
		// 0x6 ///////////////////////////////////////////////////////////////////////////
		
		case 0x60:
		{
			// MOV H,B
			cpuState->regH = cpuState->regB;
			break;
		}
		
		case 0x61:
		{
			// MOV H,C
			cpuState->regH = cpuState->regC;
			break;
		}
		
		case 0x62:
		{
			// MOV H,D
			cpuState->regH = cpuState->regD;
			break;
		}
		
		case 0x63:
		{
			// MOV H,E
			cpuState->regH = cpuState->regE;
			break;
		}
		
		case 0x64:
		{
			// MOV H,H
			cpuState->regH = cpuState->regH;
			break;
		}
		
		case 0x65:
		{
			// MOV H,L
			cpuState->regH = cpuState->regL;
			break;
		}
		
		case 0x66:
		{
			// MOV H,M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->regH = cpuState->memory[pairHL];
			break;
		}
		
		case 0x67:
		{
			// MOV H,A
			cpuState->regH = cpuState->regA;
			break;
		}
		
		case 0x68:
		{
			// MOV L,B
			cpuState->regL = cpuState->regB;
			break;
		}
		
		case 0x69:
		{
			// MOV L,C
			cpuState->regL = cpuState->regC;
			break;
		}
		
		case 0x6a:
		{
			// MOV L,D
			cpuState->regL = cpuState->regD;
			break;
		}
		
		case 0x6b:
		{
			// MOV L,E
			cpuState->regL = cpuState->regE;
			break;
		}
		
		case 0x6c:
		{
			// MOV L,H
			cpuState->regL = cpuState->regH;
			break;
		}
		
		case 0x6d:
		{
			// MOV L,L
			cpuState->regL = cpuState->regL;
			break;
		}
		
		case 0x6e:
		{
			// MOV L,M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->regL = cpuState->memory[pairHL];
			break;
		}
		
		case 0x6f:
		{
			// MOV L,A
			cpuState->regL = cpuState->regA;
			break;
		}
		
		// 0x7 ///////////////////////////////////////////////////////////////////////////
		
		case 0x70:
		{
			// MOV M,B
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->memory[pairHL] = cpuState->regB;
			break;
		}
		
		case 0x71:
		{
			// MOV M,C
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->memory[pairHL] = cpuState->regC;
			break;
		}
		
		case 0x72:
		{
			// MOV M,D
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->memory[pairHL] = cpuState->regD;
			break;
		}
		
		case 0x73:
		{
			// MOV M,E
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->memory[pairHL] = cpuState->regE;
			break;
		}
		
		case 0x74:
		{
			// MOV M,H
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->memory[pairHL] = cpuState->regH;
			break;
		}
		
		case 0x75:
		{
			// MOV M,L
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->memory[pairHL] = cpuState->regL;
			break;
		}
		
		case 0x76:
		{
			// HLT
			exit(0);
			break;
		}
		
		case 0x77:
		{
			// MOV M,A
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->memory[pairHL] = cpuState->regA;
			break;
		}
		
		case 0x78:
		{
			// MOV A,B
			cpuState->regA = cpuState->regB;
			break;
		}
		
		case 0x79:
		{
			// MOV A,C
			cpuState->regA = cpuState->regC;
			break;
		}
		
		case 0x7a:
		{
			// MOV A,D
			cpuState->regA = cpuState->regD;
			break;
		}
		
		case 0x7b:
		{
			// MOV A,E
			cpuState->regA = cpuState->regE;
			break;
		}
		
		case 0x7c:
		{
			// MOV A,H
			cpuState->regA = cpuState->regH;
			break;
		}
		
		case 0x7d:
		{
			// MOV A,L
			cpuState->regA = cpuState->regL;
			break;
		}
		
		case 0x7e:
		{
			// MOV A,M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			cpuState->regA = cpuState->memory[pairHL];
			break;
		}
		
		case 0x7f:
		{
			// MOV A,A
			cpuState->regA = cpuState->regA;
			break;
		}
		
		// 0x8 ///////////////////////////////////////////////////////////////////////////
		
		case 0x80:
		{
			ASSERT(false);
			break;
		}
		
		case 0x81:
		{
			ASSERT(false);
			break;
		}
		
		case 0x82:
		{
			ASSERT(false);
			break;
		}
		
		case 0x83:
		{
			ASSERT(false);
			break;
		}
		
		case 0x84:
		{
			ASSERT(false);
			break;
		}
		
		case 0x85:
		{
			ASSERT(false);
			break;
		}
		
		case 0x86:
		{
			ASSERT(false);
			break;
		}
		
		case 0x87:
		{
			ASSERT(false);
			break;
		}
		
		case 0x88:
		{
			ASSERT(false);
			break;
		}
		
		case 0x89:
		{
			ASSERT(false);
			break;
		}
		
		case 0x8a:
		{
			ASSERT(false);
			break;
		}
		
		case 0x8b:
		{
			ASSERT(false);
			break;
		}
		
		case 0x8c:
		{
			ASSERT(false);
			break;
		}
		
		case 0x8d:
		{
			ASSERT(false);
			break;
		}
		
		case 0x8e:
		{
			ASSERT(false);
			break;
		}
		
		case 0x8f:
		{
			ASSERT(false);
			break;
		}
		
		// 0x9 ///////////////////////////////////////////////////////////////////////////
		
		case 0x90:
		{
			ASSERT(false);
			break;
		}
		
		case 0x91:
		{
			ASSERT(false);
			break;
		}
		
		case 0x92:
		{
			ASSERT(false);
			break;
		}
		
		case 0x93:
		{
			ASSERT(false);
			break;
		}
		
		case 0x94:
		{
			ASSERT(false);
			break;
		}
		
		case 0x95:
		{
			ASSERT(false);
			break;
		}
		
		case 0x96:
		{
			ASSERT(false);
			break;
		}
		
		case 0x97:
		{
			ASSERT(false);
			break;
		}
		
		case 0x98:
		{
			ASSERT(false);
			break;
		}
		
		case 0x99:
		{
			ASSERT(false);
			break;
		}
		
		case 0x9a:
		{
			ASSERT(false);
			break;
		}
		
		case 0x9b:
		{
			ASSERT(false);
			break;
		}
		
		case 0x9c:
		{
			ASSERT(false);
			break;
		}
		
		case 0x9d:
		{
			ASSERT(false);
			break;
		}
		
		case 0x9e:
		{
			ASSERT(false);
			break;
		}
		
		case 0x9f:
		{
			ASSERT(false);
			break;
		}
		
		// 0xa ///////////////////////////////////////////////////////////////////////////
		
		case 0xa0:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa1:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa2:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa3:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa4:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa5:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa6:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa7:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa8:
		{
			ASSERT(false);
			break;
		}
		
		case 0xa9:
		{
			ASSERT(false);
			break;
		}
		
		case 0xaa:
		{
			ASSERT(false);
			break;
		}
		
		case 0xab:
		{
			ASSERT(false);
			break;
		}
		
		case 0xac:
		{
			ASSERT(false);
			break;
		}
		
		case 0xad:
		{
			ASSERT(false);
			break;
		}
		
		case 0xae:
		{
			ASSERT(false);
			break;
		}
		
		case 0xaf:
		{
			ASSERT(false);
			break;
		}
		
		// 0xb ///////////////////////////////////////////////////////////////////////////
		
		case 0xb0:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb1:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb2:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb3:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb4:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb5:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb6:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb7:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb8:
		{
			ASSERT(false);
			break;
		}
		
		case 0xb9:
		{
			ASSERT(false);
			break;
		}
		
		case 0xba:
		{
			ASSERT(false);
			break;
		}
		
		case 0xbb:
		{
			ASSERT(false);
			break;
		}
		
		case 0xbc:
		{
			ASSERT(false);
			break;
		}
		
		case 0xbd:
		{
			ASSERT(false);
			break;
		}
		
		case 0xbe:
		{
			ASSERT(false);
			break;
		}
		
		case 0xbf:
		{
			ASSERT(false);
			break;
		}
		
		// 0xc ///////////////////////////////////////////////////////////////////////////
		
		case 0xc0:
		{
			ASSERT(false);
			break;
		}
		
		case 0xc1:
		{
			ASSERT(false);
			break;
		}
		
		case 0xc2:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xc3:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xc4:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xc5:
		{
			ASSERT(false);
			break;
		}
		
		case 0xc6:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xc7:
		{
			ASSERT(false);
			break;
		}
		
		case 0xc8:
		{
			ASSERT(false);
			break;
		}
		
		case 0xc9:
		{
			ASSERT(false);
			break;
		}
		
		case 0xca:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xcb:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xcc:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xcd:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xce:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xcf:
		{
			ASSERT(false);
			break;
		}
		
		// 0xd ///////////////////////////////////////////////////////////////////////////
		
		case 0xd0:
		{
			ASSERT(false);
			break;
		}
		
		case 0xd1:
		{
			ASSERT(false);
			break;
		}
		
		case 0xd2:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xd3:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xd4:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xd5:
		{
			ASSERT(false);
			break;
		}
		
		case 0xd6:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xd7:
		{
			ASSERT(false);
			break;
		}
		
		case 0xd8:
		{
			ASSERT(false);
			break;
		}
		
		case 0xd9:
		{
			ASSERT(false);
			break;
		}
		
		case 0xda:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xdb:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xdc:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xdd:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xde:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xdf:
		{
			ASSERT(false);
			break;
		}
		
		// 0xe ///////////////////////////////////////////////////////////////////////////
		
		case 0xe0:
		{
			ASSERT(false);
			break;
		}
		
		case 0xe1:
		{
			ASSERT(false);
			break;
		}
		
		case 0xe2:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xe3:
		{
			ASSERT(false);
			break;
		}
		
		case 0xe4:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xe5:
		{
			ASSERT(false);
			break;
		}
		
		case 0xe6:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xe7:
		{
			ASSERT(false);
			break;
		}
		
		case 0xe8:
		{
			ASSERT(false);
			break;
		}
		
		case 0xe9:
		{
			ASSERT(false);
			break;
		}
		
		case 0xea:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xeb:
		{
			ASSERT(false);
			break;
		}
		
		case 0xec:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xed:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xee:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xef:
		{
			ASSERT(false);
			break;
		}
		
		// 0xf ///////////////////////////////////////////////////////////////////////////
		
		case 0xf0:
		{
			ASSERT(false);
			break;
		}
		
		case 0xf1:
		{
			ASSERT(false);
			break;
		}
		
		case 0xf2:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xf3:
		{
			ASSERT(false);
			break;
		}
		
		case 0xf4:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xf5:
		{
			ASSERT(false);
			break;
		}
		
		case 0xf6:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xf7:
		{
			ASSERT(false);
			break;
		}
		
		case 0xf8:
		{
			ASSERT(false);
			break;
		}
		
		case 0xf9:
		{
			ASSERT(false);
			break;
		}
		
		case 0xfa:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xfb:
		{
			ASSERT(false);
			break;
		}
		
		case 0xfc:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xfd:
		{
			opBytes = 3;
			ASSERT(false);
			break;
		}
		
		case 0xfe:
		{
			opBytes = 2;
			ASSERT(false);
			break;
		}
		
		case 0xff:
		{
			ASSERT(false);
			break;
		}
		
		default:
		{
			// NOP
			break;
		}
	}
	
	cpuState->programCounter += opBytes;
	
	char cpuPrint[64] = {};
	sprintf_s(cpuPrint, sizeof(cpuPrint), "\tCPU FLAGS:\nC=%d,P=%d,A=%d,S=%d,Z=%d\n", cpuState->regF.c, cpuState->regF.p, cpuState->regF.a, cpuState->regF.s, cpuState->regF.z);
	OutputDebugStringA(cpuPrint);
	
	sprintf_s(cpuPrint, sizeof(cpuPrint), "\tREGISTERS:\nA=%02x,B=%02x,C=%02x,D=%02x,E=%02x,H=%02x,L=%02x,SP=%04x\n\n", cpuState->regA, cpuState->regB, cpuState->regC, cpuState->regD, cpuState->regE, cpuState->regH, cpuState->regL, cpuState->stackPointer);
	OutputDebugStringA(cpuPrint);
}


s32 CALLBACK WinMain(HINSTANCE currInstance, HINSTANCE prevInstance, LPSTR cmdLine, s32 showCode)
{
	CPUState cpuState = {};
	cpuState.regF.unused1 = 0;
	cpuState.regF.unused2 = 0;
	cpuState.regF.unused3 = 1;
	cpuState.programCounter = 0;
	cpuState.memory = (u8 *)VirtualAlloc(0, 0x10000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	char *romFilename = "invaders.eer";
	u16 romSize = 0x2000;
	
	HANDLE romHandle =  CreateFileA(romFilename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (romHandle != INVALID_HANDLE_VALUE)
	{
		if (cpuState.memory)
		{
			DWORD bytesRead;
			ReadFile(romHandle, cpuState.memory, romSize, &bytesRead, 0);
		}
		
		CloseHandle(romHandle);
	}
	
	unsigned char *castedMemContents = (unsigned char *)cpuState.memory;
	
#if EMU8080_INTERNAL
	/*
 u16 calledInstructions[0x100] = {};
 while (cpuState.programCounter < romSize)
 {
 
 // PRINT ROM HEX
 char tempPrint[32];
 sprintf_s(tempPrint, sizeof(tempPrint), "%02x %02x %02x %02x %02x %02x %02x %02x\n", printContents[i], printContents[i+1], printContents[i+2], printContents[i+3], printContents[i+4], printContents[i+5], printContents[i+6], printContents[i+7]);
 OutputDebugStringA(tempPrint);
 i += 8;
 tempPrint[0] = 0;
 
 // PRINT ROM ASM
 unsigned char *opCode = &castedMemContents[cpuState.programCounter];
 char tempPrint[32] = {};
 sprintf_s(tempPrint, sizeof(tempPrint), "%04x ", cpuState.programCounter);
 OutputDebugStringA(tempPrint);
 
 cpuState.programCounter += PrintDisassembly(tempPrint, opCode, &calledInstructions);
 }
 */
#endif
	
	while (true)
	{
		Emulate(&cpuState, castedMemContents);
	}
	
	/*
 for (u16 i = 0x0000; i != 0x0100; ++i)
 {
 if (calledInstructions[i] != 0)
 {
 char tempPrint[32] = {};
 sprintf_s(tempPrint, sizeof(tempPrint), "%02x ", i);
 OutputDebugStringA(tempPrint);
 
 sprintf_s(tempPrint, sizeof(tempPrint), "%u\n", calledInstructions[i]);
 OutputDebugStringA(tempPrint);
 }
 }
 */
	
	VirtualFree(cpuState.memory, 0, MEM_RELEASE);
	return 0;
}


