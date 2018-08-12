/*
Project: Intel 8080 CPU Emulator
File: 8080emu.cpp
Author: Brock Salmon
Notice: (C) Copyright 2018 by Brock Salmon. All Rights Reserved.
*/

#include "8080emu.h"

internal_func u8 SetZeroFlag(u16 value)
{
	return ((value & 0xff) == 0x00);
}

internal_func u8 SetSignFlag(u16 value)
{
	return ((value & (1<<7)) == (1<<7));
}

internal_func u8 SetAuxiliaryFlag(u8 a, u16 b)
{
	return ((a & 0x0f) > (b & 0x000f));
}

internal_func u8 SetParityFlag(u8 x)
{
	u8 bitMask = 1;
	u8 parityByteCount = 0;
	
	while (bitMask)
	{
		if (x & bitMask)
		{
			parityByteCount++;
		}
		bitMask <<= 1;
	}
	
	return !(parityByteCount % 2);
}

internal_func u8 BuildPSW(CPUState *cpuState)
{
	u8 psw = 0x00;
	
	psw = cpuState->regF.s << 7 | cpuState->regF.z << 6 
		| cpuState->regF.unused1 << 5 | cpuState->regF.a << 4
		| cpuState->regF.unused2 << 3 | cpuState->regF.p << 2
		| cpuState->regF.unused3 << 1 | cpuState->regF.c;
	
	return psw;
}

internal_func void SafeMemWrite(CPUState *cpuState, u16 adr, u8 value)
{
	if (!(adr < 0x2000) && !(adr >= 0x4000))
	{
		cpuState->memory[adr] = value;
	}
	
	// NOTE(bSalmon): Used for debugging VRAM issues
	/*
 #if EMU8080_INTERNAL
  if (adr >= 0x2400 && adr < 0x4000)
  {
   char vramPrint[128] = {};
   sprintf_s(vramPrint, sizeof(vramPrint), "Writing %02x to VRAM Adr: %04x\n", value, adr);
   OutputDebugStringA(vramPrint);
  }
 #endif*/
}

internal_func void ProcessMachineKeyDown(u8 *port, u8 key)
{
	switch(key)
	{
		case 0:
		{
			*port |= 0x01;
			break;
		}
		
		case 1:
		{
			*port |= (1<<1);
			break;
		}
		
		case 2:
		{
			*port |= (1<<2);
			break;
		}
		
		case 3:
		{
			*port |= (1<<3);
			break;
		}
		
		case 4:
		{
			*port |= (1<<4);
			break;
		}
		
		case 5:
		{
			*port |= (1<<5);
			break;
		}
		
		case 6:
		{
			*port |= (1<<6);
			break;
		}
		
		case 7:
		{
			*port |= (1<<7);
			break;
		}
		
		default:
		{
			break;
		}
	}
}

internal_func void ProcessMachineKeyUp(u8 *port, u8 key)
{
	switch(key)
	{
		case 0:
		{
			*port &= ~0x01;
			break;
		}
		
		case 1:
		{
			*port &= ~(1<<1);
			break;
		}
		
		case 2:
		{
			*port &= ~(1<<2);
			break;
		}
		
		case 3:
		{
			*port &= ~(1<<3);
			break;
		}
		
		case 4:
		{
			*port &= ~(1<<4);
			break;
		}
		
		case 5:
		{
			*port &= ~(1<<5);
			break;
		}
		
		case 6:
		{
			*port &= ~(1<<6);
			break;
		}
		
		case 7:
		{
			*port &= ~(1<<7);
			break;
		}
		
		default:
		{
			break;
		}
	}
}

internal_func void HandleINInst(CPUState *cpuState, u8 *opCode)
{
	switch(opCode[1])
	{
		case 0x00:
		{
			cpuState->regA = 0x01;
			break;
		}
		
		case 0x01:
		{
			cpuState->regA = cpuState->inputPort1;
			break;
		}
		
		case 0x02:
		{
			cpuState->regA = cpuState->inputPort2;
			break;
		}
		
		case 0x03:
		{
			u16 v = (cpuState->shift1 << 8) | cpuState->shift0;
			cpuState->regA = (v >> (8 - cpuState->shiftOffset)) & 0xff;
			break;
		}
		default:
		{
			break;
		}
	}
}

internal_func void HandleOUTInst(CPUState *cpuState, u8 *opCode)
{
	switch(opCode[1])
	{
		case 0x02:
		{
			cpuState->shiftOffset = cpuState->regA & 0x07;
			break;
		}
		case 0x04:
		{
			cpuState->shift0 = cpuState->shift1;
			cpuState->shift1 = cpuState->regA;
			break;
		}
		default:
		{
			break;
		}
	}
}

internal_func void RenderVideoMemContents(BackBuffer *backBuffer, CPUState *cpuState)
{
	// Pixel Colours
	u32 white = 0xFFFFFFFF;
	u32 red = 0xFFFF0000;
	u32 green = 0xFF00FF00;
	u32 black = 0xFF000000;
	
	u8 *screenBuffer = (u8 *)backBuffer->memory;
	u8 *videoBuffer = &cpuState->memory[0x2400];
	for (s32 x = 0; x < backBuffer->width; ++x)
	{
		for (s32 y = 0; y < backBuffer->height; y += 8)
		{
			u8 pixel8080 = videoBuffer[(x * (backBuffer->height / 8)) + (y / 8)];
			
			s32 offset = ((backBuffer->height - 1) - y) * (backBuffer->pitch) + (x * 4);
			u32 *outputPixel = (u32 *)(&screenBuffer[offset]);
			
			for (s32 bit = 0; bit < 8; bit++)
			{
				if (((1<<bit) & pixel8080) != 0)
				{
					if ((backBuffer->height-y) >= (backBuffer->height/8) && (backBuffer->height-y) < (backBuffer->height/4))
					{
						// Scores
						*outputPixel = red;
					}
					else if ((backBuffer->height-y) >= (s32)(backBuffer->height/1.39f) &&
							 (backBuffer->height-y) < (s32)(backBuffer->height/1.06667f))
					{
						// Player and Shields
						*outputPixel = green;
					}
					else if ((backBuffer->height-y) >= (s32)(backBuffer->height/1.05f) &&
							 x >= (backBuffer->width/16) && 
							 x < (s32)(backBuffer->width/1.91f))
					{
						// Lives indicator
						*outputPixel = green;
					}
					else
					{
						*outputPixel = white;
					}
				}
				else
				{
					*outputPixel = black;
				}
				
				outputPixel -= backBuffer->width;
			}
		}
	}
}

internal_func void Interrupt(CPUState *cpuState, u8 interruptNum, u64 *cycles)
{
	SafeMemWrite(cpuState, cpuState->stackPointer - 1, (cpuState->programCounter >> 8) & 0xff);
	SafeMemWrite(cpuState, cpuState->stackPointer - 2, cpuState->programCounter & 0xff);
	cpuState->stackPointer -= 2;
	*cycles += 11;
	
	cpuState->programCounter = 8 * interruptNum;
	
	cpuState->enableInterrupt = false;
	*cycles += 4;
}

internal_func void Emulate(CPUState *cpuState, unsigned char *castedMem, u64 *cycles)
{
	u8 *opCode = &castedMem[cpuState->programCounter];
	b32 altCycles = false;
	
	cpuState->programCounter++;
	
	switch(*opCode)
	{
		// 0x0 ///////////////////////////////////////////////////////////////////////////
		
		case 0x01:
		{
			// LXI B,D16
			cpuState->regB = opCode[2];
			cpuState->regC = opCode[1];
			cpuState->programCounter += 2;
			break;
		}
		
		case 0x02:
		{
			// STAX B
			u16 pairBC = (cpuState->regB << 8) | cpuState->regC;
			SafeMemWrite(cpuState, pairBC, cpuState->regA);
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
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regB, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regB = result;
			break;
		}
		
		case 0x05:
		{
			// DCR B
			u8 result = cpuState->regB - 1;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regB, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regB = result;
			break;
		}
		
		case 0x06:
		{
			// MVI B,a8
			cpuState->regB = opCode[1];
			cpuState->programCounter++;
			break;
		}
		
		case 0x07:
		{
			// RLC
			u8 result = cpuState->regA;
			cpuState->regA = ((result & (1<<7)) >> 7) | (result << 1);
			cpuState->regF.c = ((result & (1<<7)) == (1<<7));
			break;
		}
		
		case 0x09:
		{
			// DAD B
			u32 pairBC = (cpuState->regB << 8) | cpuState->regC;
			u32 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u32 result = pairHL + pairBC;
			cpuState->regH = (result >> 8) & 0xff;
			cpuState->regL = result & 0xff;
			cpuState->regF.c = ((result & 0xffff0000) != 0);
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
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regC, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regC = result;
			break;
		}
		
		case 0x0d:
		{
			// DCR C
			u8 result = cpuState->regC - 1;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regC, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regC = result;
			break;
		}
		
		case 0x0e:
		{
			// MVI C,a8
			cpuState->regC = opCode[1];
			cpuState->programCounter++;
			break;
		}
		
		case 0x0f:
		{
			// RRC
			u8 result = cpuState->regA;
			cpuState->regA = ((result & 0x01) << 7) | (result >> 1);
			cpuState->regF.c = ((result & 0x01) == 0x01);
			break;
		}
		
		// 0x1 ///////////////////////////////////////////////////////////////////////////
		
		case 0x11:
		{
			// LXI D,D16
			cpuState->regD = opCode[2];
			cpuState->regE = opCode[1];
			cpuState->programCounter += 2;
			break;
		}
		
		case 0x12:
		{
			// STAX D
			u16 pairDE = (cpuState->regD << 8) | cpuState->regE;
			SafeMemWrite(cpuState, pairDE, cpuState->regA);
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
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regD, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regD = result;
			break;
		}
		
		case 0x15:
		{
			// DCR D
			u8 result = cpuState->regD - 1;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regD, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regD = result;
			break;
		}
		
		case 0x16:
		{
			// MVI D,a8
			cpuState->regD = opCode[1];
			cpuState->programCounter++;
			break;
		}
		
		case 0x17:
		{
			// RAL
			u8 result = cpuState->regA;
			cpuState->regA = (result << 1) | cpuState->regF.c;
			cpuState->regF.c = ((result & (1<<7)) == (1<<7));
			break;
		}
		
		case 0x19:
		{
			// DAD D
			u32 pairDE = (cpuState->regD << 8) | cpuState->regE;
			u32 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u32 result = pairHL + pairDE;
			cpuState->regH = (result >> 8) & 0xff;
			cpuState->regL = result & 0xff;
			cpuState->regF.c = ((result & 0xffff0000) != 0);
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
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regE, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regE = result;
			break;
		}
		
		case 0x1d:
		{
			// DCR E
			u8 result = cpuState->regE - 1;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regE, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regE = result;
			break;
		}
		
		case 0x1e:
		{
			// MVI E,a8
			cpuState->regE = opCode[1];
			cpuState->programCounter++;
			break;
		}
		
		case 0x1f:
		{
			// RAR
			u8 result = cpuState->regA;
			cpuState->regA = (cpuState->regF.c << 7) | (result >> 1);
			cpuState->regF.c = ((result & 0x01) == 0x01);
			break;
		}
		
		// 0x2 ///////////////////////////////////////////////////////////////////////////
		
		case 0x21:
		{
			// LXI H,D16
			cpuState->regH = opCode[2];
			cpuState->regL = opCode[1];
			cpuState->programCounter += 2;
			break;
		}
		
		case 0x22:
		{
			// SHLD a16
			u16 adr = (opCode[2] << 8) | opCode[1];
			SafeMemWrite(cpuState, adr, cpuState->regL);
			adr++;
			SafeMemWrite(cpuState, adr, cpuState->regH);
			cpuState->programCounter += 2;
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
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regH, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regH = result;
			break;
		}
		
		case 0x25:
		{
			// DCR H
			u8 result = cpuState->regH - 1;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regH, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regH = result;
			break;
		}
		
		case 0x26:
		{
			// MVI H,a8
			cpuState->regH = opCode[1];
			cpuState->programCounter++;
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
				cpuState->regA += 0x06;
				cpuState->regF.a = 1;
			}
			else
			{
				cpuState->regF.a = 0;
			}
			
			u8 highBits = cpuState->regA;
			highBits >>= 4;
			if (cpuState->regF.c || (highBits > 0x09))
			{
				highBits += 0x06;
				highBits <<= 4;
				cpuState->regA <<= 4;
				cpuState->regA >>= 4;
				cpuState->regA |= highBits;
				cpuState->regF.c = 1;
			}
			
			cpuState->regF.z = SetZeroFlag(cpuState->regA);
			cpuState->regF.s = SetSignFlag(cpuState->regA);
			cpuState->regF.p = SetParityFlag(cpuState->regA);
			
			break;
		}
		
		case 0x29:
		{
			// DAD H
			u32 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u32 result = pairHL + pairHL;
			cpuState->regH = (result >> 8) & 0xff;
			cpuState->regL = result & 0xff;
			cpuState->regF.c = ((result & 0xffff0000) != 0);
			break;
		}
		
		case 0x2a:
		{
			// LHLD a16
			u16 adr = (opCode[2] << 8) | opCode[1];
			cpuState->regL = cpuState->memory[adr];
			adr++;
			cpuState->regH = cpuState->memory[adr];
			cpuState->programCounter += 2;
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
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regL, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regL = result;
			break;
		}
		
		case 0x2d:
		{
			// DCR L
			u8 result = cpuState->regL - 1;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regL, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regL = result;
			break;
		}
		
		case 0x2e:
		{
			// MVI L,a8
			cpuState->regL = opCode[1];
			cpuState->programCounter++;
			break;
		}
		
		case 0x2f:
		{
			// CMA
			cpuState->regA = ~cpuState->regA;
			break;
		}
		
		// 0x3 ///////////////////////////////////////////////////////////////////////////
		
		case 0x31:
		{
			// LXI SP,D16
			cpuState->stackPointer = (opCode[2] << 8) | opCode[1];
			cpuState->programCounter += 2;
			break;
		}
		
		case 0x32:
		{
			// STA a16
			u16 adr = (opCode[2] << 8) | opCode[1];
			SafeMemWrite(cpuState, adr, cpuState->regA);
			cpuState->programCounter += 2;
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
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->memory[pairHL], result);
			cpuState->regF.p = SetParityFlag(result);
			SafeMemWrite(cpuState, pairHL, result);
			break;
		}
		
		case 0x35:
		{
			// DCR M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u8 result = cpuState->memory[pairHL] - 1;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->memory[pairHL], result);
			cpuState->regF.p = SetParityFlag(result);
			SafeMemWrite(cpuState, pairHL, result);
			break;
		}
		
		case 0x36:
		{
			// MVI M,a8
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			SafeMemWrite(cpuState, pairHL, opCode[1]);
			cpuState->programCounter++;
			break;
		}
		
		case 0x37:
		{
			// STC
			cpuState->regF.c = 1;
			break;
		}
		
		case 0x39:
		{
			// DAD SP
			u32 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u32 result = pairHL + cpuState->stackPointer;
			cpuState->regH = (result >> 8) & 0xff;
			cpuState->regL = result & 0xff;
			cpuState->regF.c = ((result & 0xffff0000) != 0);
			break;
		}
		
		case 0x3a:
		{
			// LDA a16
			u16 adr = (opCode[2] << 8) | opCode[1];
			cpuState->regA = cpuState->memory[adr];
			cpuState->programCounter += 2;
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
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regA = result;
			break;
		}
		
		case 0x3d:
		{
			// DCR A
			u8 result = cpuState->regA - 1;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regA = result;
			break;
		}
		
		case 0x3e:
		{
			// MVI A,a8
			cpuState->regA = opCode[1];
			cpuState->programCounter++;
			break;
		}
		
		case 0x3f:
		{
			// CMC
			if (cpuState->regF.c)
			{
				cpuState->regF.c = 0;
			}
			else
			{
				cpuState->regF.c = 1;
			}
			
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
			SafeMemWrite(cpuState, pairHL, cpuState->regB);
			break;
		}
		
		case 0x71:
		{
			// MOV M,C
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			SafeMemWrite(cpuState, pairHL, cpuState->regC);
			break;
		}
		
		case 0x72:
		{
			// MOV M,D
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			SafeMemWrite(cpuState, pairHL, cpuState->regD);
			break;
		}
		
		case 0x73:
		{
			// MOV M,E
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			SafeMemWrite(cpuState, pairHL, cpuState->regE);
			break;
		}
		
		case 0x74:
		{
			// MOV M,H
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			SafeMemWrite(cpuState, pairHL, cpuState->regH);
			break;
		}
		
		case 0x75:
		{
			// MOV M,L
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			SafeMemWrite(cpuState, pairHL, cpuState->regL);
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
			SafeMemWrite(cpuState, pairHL, cpuState->regA);
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
			// ADD B
			u16 result = cpuState->regA + cpuState->regB;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x81:
		{
			// ADD C
			u16 result = cpuState->regA + cpuState->regC;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x82:
		{
			// ADD D
			u16 result = cpuState->regA + cpuState->regD;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x83:
		{
			// ADD E
			u16 result = cpuState->regA + cpuState->regE;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x84:
		{
			// ADD H
			u16 result = cpuState->regA + cpuState->regH;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x85:
		{
			// ADD L
			u16 result = cpuState->regA + cpuState->regL;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x86:
		{
			// ADD M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = cpuState->regA + cpuState->memory[pairHL];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x87:
		{
			// ADD A
			u16 result = cpuState->regA + cpuState->regA;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x88:
		{
			// ADC B
			u16 result = cpuState->regA + (cpuState->regB + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x89:
		{
			// ADC C
			u16 result = cpuState->regA + (cpuState->regC + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x8a:
		{
			// ADC D
			u16 result = cpuState->regA + (cpuState->regD + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x8b:
		{
			// ADC E
			u16 result = cpuState->regA + (cpuState->regE + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x8c:
		{
			// ADC H
			u16 result = cpuState->regA + (cpuState->regH + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x8d:
		{
			// ADC L
			u16 result = cpuState->regA + (cpuState->regL + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x8e:
		{
			// ADC M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = cpuState->regA + (cpuState->memory[pairHL] + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x8f:
		{
			// ADC A
			u16 result = cpuState->regA + (cpuState->regA + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result & 0xff);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		// 0x9 ///////////////////////////////////////////////////////////////////////////
		
		case 0x90:
		{
			// SUB B
			u16 result = cpuState->regA - cpuState->regB;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x91:
		{
			// SUB C
			u16 result = cpuState->regA - cpuState->regC;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x92:
		{
			// SUB D
			u16 result = cpuState->regA - cpuState->regD;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x93:
		{
			// SUB E
			u16 result = cpuState->regA - cpuState->regE;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x94:
		{
			// SUB H
			u16 result = cpuState->regA - cpuState->regH;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x95:
		{
			// SUB L
			u16 result = cpuState->regA - cpuState->regL;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x96:
		{
			// SUB M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = cpuState->regA - cpuState->memory[pairHL];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x97:
		{
			// SUB A
			u16 result = cpuState->regA - cpuState->regA;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x98:
		{
			// SBB B
			u16 result = cpuState->regA - (cpuState->regB + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x99:
		{
			// SBB C
			u16 result = cpuState->regA - (cpuState->regC + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x9a:
		{
			// SBB D
			u16 result = cpuState->regA - (cpuState->regD + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x9b:
		{
			// SBB E
			u16 result = cpuState->regA - (cpuState->regE + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x9c:
		{
			// SBB H
			u16 result = cpuState->regA - (cpuState->regH + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x9d:
		{
			// SBB L
			u16 result = cpuState->regA - (cpuState->regL + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x9e:
		{
			// SBB M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = cpuState->regA - (cpuState->memory[pairHL] + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		case 0x9f:
		{
			// SBB A
			u16 result = cpuState->regA - (cpuState->regA + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			break;
		}
		
		// 0xa ///////////////////////////////////////////////////////////////////////////
		
		case 0xa0:
		{
			// ANA B
			u8 result = cpuState->regA & cpuState->regB;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa1:
		{
			// ANA C
			u8 result = cpuState->regA & cpuState->regC;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa2:
		{
			// ANA D
			u8 result = cpuState->regA & cpuState->regD;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa3:
		{
			// ANA E
			u8 result = cpuState->regA & cpuState->regE;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa4:
		{
			// ANA H
			u8 result = cpuState->regA & cpuState->regH;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa5:
		{
			// ANA L
			u8 result = cpuState->regA & cpuState->regL;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa6:
		{
			// ANA M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u8 result = cpuState->regA & cpuState->memory[pairHL];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa7:
		{
			// ANA A
			u8 result = cpuState->regA & cpuState->regA;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa8:
		{
			// XRA B
			u8 result = cpuState->regA ^ cpuState->regB;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xa9:
		{
			// XRA C
			u8 result = cpuState->regA ^ cpuState->regC;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xaa:
		{
			// XRA D
			u8 result = cpuState->regA ^ cpuState->regD;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xab:
		{
			// XRA E
			u8 result = cpuState->regA ^ cpuState->regE;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xac:
		{
			// XRA H
			u8 result = cpuState->regA ^ cpuState->regH;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xad:
		{
			// XRA L
			u8 result = cpuState->regA ^ cpuState->regL;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xae:
		{
			// XRA M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u8 result = cpuState->regA ^ cpuState->memory[pairHL];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xaf:
		{
			// XRA A (Zero Accumulator)
			u8 result = cpuState->regA ^ cpuState->regA;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		// 0xb ///////////////////////////////////////////////////////////////////////////
		
		case 0xb0:
		{
			// ORA B
			u8 result = cpuState->regA | cpuState->regB;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xb1:
		{
			// ORA C
			u8 result = cpuState->regA | cpuState->regC;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xb2:
		{
			// ORA D
			u8 result = cpuState->regA | cpuState->regD;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xb3:
		{
			// ORA E
			u8 result = cpuState->regA | cpuState->regE;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xb4:
		{
			// ORA H
			u8 result = cpuState->regA | cpuState->regH;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xb5:
		{
			// ORA L
			u8 result = cpuState->regA | cpuState->regL;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xb6:
		{
			// ORA M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u8 result = cpuState->regA | cpuState->memory[pairHL];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xb7:
		{
			// ORA A
			u8 result = cpuState->regA | cpuState->regA;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			break;
		}
		
		case 0xb8:
		{
			// CMP B
			u16 result = cpuState->regA - cpuState->regB;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			break;
		}
		
		case 0xb9:
		{
			// CMP C
			u16 result = cpuState->regA - cpuState->regC;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			break;
		}
		
		case 0xba:
		{
			// CMP D
			u16 result = cpuState->regA - cpuState->regD;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			break;
		}
		
		case 0xbb:
		{
			// CMP E
			u16 result = cpuState->regA - cpuState->regE;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			break;
		}
		
		case 0xbc:
		{
			// CMP H
			u16 result = cpuState->regA - cpuState->regH;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			break;
		}
		
		case 0xbd:
		{
			// CMP L
			u16 result = cpuState->regA - cpuState->regL;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			break;
		}
		
		case 0xbe:
		{
			// CMP M
			u16 pairHL = (cpuState->regH << 8) | cpuState->regL;
			u16 result = cpuState->regA - cpuState->memory[pairHL];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			break;
		}
		
		case 0xbf:
		{
			// CMP A
			u16 result = cpuState->regA - cpuState->regA;
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			break;
		}
		
		// 0xc ///////////////////////////////////////////////////////////////////////////
		
		case 0xc0:
		{
			// RNZ
			if (!cpuState->regF.z)
			{
				cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
				cpuState->stackPointer += 2;
			}
			else
			{
				altCycles = true;
				*cycles += 5;
			}
			break;
		}
		
		case 0xc1:
		{
			// POP B
			cpuState->regC = cpuState->memory[cpuState->stackPointer];
			cpuState->regB = cpuState->memory[cpuState->stackPointer + 1];
			cpuState->stackPointer += 2;
			break;
		}
		
		case 0xc2:
		{
			// JNZ a16
			if (!cpuState->regF.z)
			{
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xc3:
		{
			// JMP a16
			cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			break;
		}
		
		case 0xc4:
		{
			// CNZ a16
			if (!cpuState->regF.z)
			{
				u16 result = cpuState->programCounter + 2;
				SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
				SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
				cpuState->stackPointer -= 2;
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				altCycles = true;
				*cycles += 11;
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xc5:
		{
			// PUSH B
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, cpuState->regB);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, cpuState->regC);
			cpuState->stackPointer -= 2;
			break;
		}
		
		case 0xc6:
		{
			// ADI a8
			u16 result = cpuState->regA + opCode[1];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			cpuState->programCounter++;
			break;
		}
		
		case 0xc7:
		{
			// RST 0
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = 0x0000;
			break;
		}
		
		case 0xc8:
		{
			// RZ
			if (cpuState->regF.z)
			{
				cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
				cpuState->stackPointer += 2;
			}
			else
			{
				altCycles = true;
				*cycles += 5;
			}
			break;
		}
		
		case 0xc9:
		{
			// RET
			cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
			cpuState->stackPointer += 2;
			break;
		}
		
		case 0xca:
		{
			// JZ a16
			if (cpuState->regF.z)
			{
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xcb:
		{
			// ALT JMP a16;
			ASSERT(false);
			break;
		}
		
		case 0xcc:
		{
			// CZ a16
			if (cpuState->regF.z)
			{
				u16 result = cpuState->programCounter + 2;
				SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
				SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
				cpuState->stackPointer -= 2;
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				altCycles = true;
				*cycles += 11;
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xcd:
		{
			// CALL a16
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			break;
		}
		
		case 0xce:
		{
			// ACI a8
			u16 result = cpuState->regA + (opCode[1] + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result;
			cpuState->programCounter++;
			break;
		}
		
		case 0xcf:
		{
			// RST 1
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = 0x0008;
			break;
		}
		
		// 0xd ///////////////////////////////////////////////////////////////////////////
		
		case 0xd0:
		{
			// RNC
			if (!cpuState->regF.c)
			{
				cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
				cpuState->stackPointer += 2;
			}
			else
			{
				altCycles = true;
				*cycles += 5;
			}
			break;
		}
		
		case 0xd1:
		{
			// POP D
			cpuState->regE = cpuState->memory[cpuState->stackPointer];
			cpuState->regD = cpuState->memory[cpuState->stackPointer + 1];
			cpuState->stackPointer += 2;
			break;
		}
		
		case 0xd2:
		{
			// JNC a16
			if (!cpuState->regF.c)
			{
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xd3:
		{
			// OUT a8
			HandleOUTInst(cpuState, opCode);
			cpuState->programCounter++;
			break;
		}
		
		case 0xd4:
		{
			// CNC a16
			if (!cpuState->regF.c)
			{
				u16 result = cpuState->programCounter + 2;
				SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
				SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
				cpuState->stackPointer -= 2;
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				altCycles = true;
				*cycles += 11;
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xd5:
		{
			// PUSH D
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, cpuState->regD);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, cpuState->regE);
			cpuState->stackPointer -= 2;
			break;
		}
		
		case 0xd6:
		{
			// SUI a8
			u16 result = cpuState->regA - opCode[1];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			cpuState->programCounter++;
			break;
		}
		
		case 0xd7:
		{
			// RST 2
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = 0x0010;
			break;
		}
		
		case 0xd8:
		{
			// RC
			if (cpuState->regF.c)
			{
				cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
				cpuState->stackPointer += 2;
			}
			else
			{
				altCycles = true;
				*cycles += 5;
			}
			break;
		}
		
		case 0xd9:
		{
			// ALT RET
			ASSERT(false);
			break;
		}
		
		case 0xda:
		{
			// JC a16
			if (cpuState->regF.c)
			{
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xdb:
		{
			// IN a8;
			HandleINInst(cpuState, opCode);
			cpuState->programCounter++;
			break;
		}
		
		case 0xdc:
		{
			// CC a16
			if (cpuState->regF.c)
			{
				u16 result = cpuState->programCounter + 2;
				SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
				SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
				cpuState->stackPointer -= 2;
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				altCycles = true;
				*cycles += 11;
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xdd:
		{
			// ALT CALL a16;
			ASSERT(false);
			break;
		}
		
		case 0xde:
		{
			// SBI a8
			u16 result = cpuState->regA - (opCode[1] + cpuState->regF.c);
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag((result & 0xff));
			cpuState->regF.c = (result > 0xff);
			cpuState->regA = result & 0xff;
			cpuState->programCounter++;
			break;
		}
		
		case 0xdf:
		{
			// RST 3
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = 0x0018;
			break;
		}
		
		// 0xe ///////////////////////////////////////////////////////////////////////////
		
		case 0xe0:
		{
			// RPO
			if (!cpuState->regF.p)
			{
				cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
				cpuState->stackPointer += 2;
			}
			else
			{
				altCycles = true;
				*cycles += 5;
			}
			break;
		}
		
		case 0xe1:
		{
			// POP H
			cpuState->regL = cpuState->memory[cpuState->stackPointer];
			cpuState->regH = cpuState->memory[cpuState->stackPointer + 1];
			cpuState->stackPointer += 2;
			break;
		}
		
		case 0xe2:
		{
			// JPO
			if (!cpuState->regF.p)
			{
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xe3:
		{
			// XTHL
			u8 tempH = cpuState->regH;
			u8 tempL = cpuState->regL;
			cpuState->regL = cpuState->memory[cpuState->stackPointer];
			cpuState->regH = cpuState->memory[cpuState->stackPointer + 1];
			SafeMemWrite(cpuState, cpuState->stackPointer, tempL);
			SafeMemWrite(cpuState, cpuState->stackPointer + 1, tempH);
			break;
		}
		
		case 0xe4:
		{
			// CPO a16
			if (!cpuState->regF.p)
			{
				u16 result = cpuState->programCounter + 2;
				SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
				SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
				cpuState->stackPointer -= 2;
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				altCycles = true;
				*cycles += 11;
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xe5:
		{
			// PUSH H
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, cpuState->regH);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, cpuState->regL);
			cpuState->stackPointer -= 2;
			break;
		}
		
		case 0xe6:
		{
			// ANI a8
			u8 result = cpuState->regA & opCode[1];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			cpuState->programCounter++;
			break;
		}
		
		case 0xe7:
		{
			// RST 4
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = 0x0020;
			break;
		}
		
		case 0xe8:
		{
			// RPE
			if (cpuState->regF.p)
			{
				cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
				cpuState->stackPointer += 2;
			}
			else
			{
				altCycles = true;
				*cycles += 5;
			}
			break;
		}
		
		case 0xe9:
		{
			// PCHL
			cpuState->programCounter = (cpuState->regH << 8) | cpuState->regL;
			break;
		}
		
		case 0xea:
		{
			// JPE
			if (cpuState->regF.p)
			{
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xeb:
		{
			// XCHG
			u8 tempH = cpuState->regH;
			u8 tempL = cpuState->regL;
			cpuState->regH = cpuState->regD;
			cpuState->regL = cpuState->regE;
			cpuState->regD = tempH;
			cpuState->regE = tempL;
			break;
		}
		
		case 0xec:
		{
			// CPE
			if (cpuState->regF.p)
			{
				u16 result = cpuState->programCounter + 2;
				SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
				SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
				cpuState->stackPointer -= 2;
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				altCycles = true;
				*cycles += 11;
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xed:
		{
			// ALT CALL a16
			ASSERT(false);
			break;
		}
		
		case 0xee:
		{
			// XRI a8
			u8 result = cpuState->regA ^ opCode[1];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			cpuState->programCounter++;
			break;
		}
		
		case 0xef:
		{
			// RST 5
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = 0x0028;
			break;
		}
		
		// 0xf ///////////////////////////////////////////////////////////////////////////
		
		case 0xf0:
		{
			// RP
			if (!cpuState->regF.s)
			{
				cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
				cpuState->stackPointer += 2;
			}
			else
			{
				altCycles = true;
				*cycles += 5;
			}
			break;
		}
		
		case 0xf1:
		{
			// POP PSW
			u8 psw = cpuState->memory[cpuState->stackPointer];
			cpuState->regF.s = (psw & (1<<7)) == (1<<7);
			cpuState->regF.z = (psw & (1<<6)) == (1<<6);
			cpuState->regF.a = (psw & (1<<4)) == (1<<4);
			cpuState->regF.p = (psw & (1<<2)) == (1<<2);
			cpuState->regF.c = (psw & 0x01) == 0x01;
			cpuState->regA = cpuState->memory[cpuState->stackPointer + 1];
			cpuState->stackPointer += 2;
			break;
		}
		
		case 0xf2:
		{
			// JP
			if (!cpuState->regF.s)
			{
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xf3:
		{
			// DI
			cpuState->enableInterrupt = 0;
			break;
		}
		
		case 0xf4:
		{
			// CP a16
			if (!cpuState->regF.s)
			{
				u16 result = cpuState->programCounter + 2;
				SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
				SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
				cpuState->stackPointer -= 2;
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				altCycles = true;
				*cycles += 11;
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xf5:
		{
			// PUSH PSW
			u8 psw = BuildPSW(cpuState);
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, cpuState->regA);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, psw);
			cpuState->stackPointer -= 2;
			break;
		}
		
		case 0xf6:
		{
			// ORI a8
			u8 result = cpuState->regA | opCode[1];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = 0;
			cpuState->regA = result;
			cpuState->programCounter++;
			break;
		}
		
		case 0xf7:
		{
			// RST 6
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = 0x0030;
			break;
		}
		
		case 0xf8:
		{
			// RM
			if (cpuState->regF.s)
			{
				cpuState->programCounter = (cpuState->memory[cpuState->stackPointer + 1] << 8) | cpuState->memory[cpuState->stackPointer];
				cpuState->stackPointer += 2;
			}
			else
			{
				altCycles = true;
				*cycles += 5;
			}
			break;
		}
		
		case 0xf9:
		{
			// SPHL
			cpuState->stackPointer = (cpuState->regH << 8) | cpuState->regL;
			break;
		}
		
		case 0xfa:
		{
			// JM
			if (cpuState->regF.s)
			{
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xfb:
		{
			// EI
			cpuState->enableInterrupt = 1;
			break;
		}
		
		case 0xfc:
		{
			// CM a16
			if (cpuState->regF.s)
			{
				u16 result = cpuState->programCounter + 2;
				SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
				SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
				cpuState->stackPointer -= 2;
				cpuState->programCounter = (opCode[2] << 8) | opCode[1];
			}
			else
			{
				altCycles = true;
				*cycles += 11;
				cpuState->programCounter += 2;
			}
			break;
		}
		
		case 0xfd:
		{
			// ALT CALL a16
			ASSERT(false);
			break;
		}
		
		case 0xfe:
		{
			// CPI a8
			u16 result = cpuState->regA - opCode[1];
			cpuState->regF.z = SetZeroFlag(result);
			cpuState->regF.s = SetSignFlag(result);
			cpuState->regF.a = SetAuxiliaryFlag(cpuState->regA, result);
			cpuState->regF.p = SetParityFlag(result);
			cpuState->regF.c = (result > 0xff);
			cpuState->programCounter++;
			break;
		}
		
		case 0xff:
		{
			// RST 7
			u16 result = cpuState->programCounter + 2;
			SafeMemWrite(cpuState, cpuState->stackPointer - 1, (result >> 8) & 0xff);
			SafeMemWrite(cpuState, cpuState->stackPointer - 2, result & 0xff);
			cpuState->stackPointer -= 2;
			cpuState->programCounter = 0x0038;
			break;
		}
		
		default:
		{
			// NOP
			break;
		}
	}
	
	if (!altCycles)
	{
		*cycles += cyclesArray[*opCode];
	}
}
