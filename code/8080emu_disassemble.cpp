/*
Project: Intel 8080 CPU Emulator
File: 8080emu_disassemble.cpp
Author: Brock Salmon

Copyright 2018 Brock Salmon

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0
   
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#if EMU8080_INTERNAL
#include <stdio.h>
internal_func void DebugPrintDisassembledROM(char *printBuffer, u8 opBytes, char *formatString, unsigned char *opCode)
{
	switch (opBytes)
	{
		case 1:
		{
			sprintf_s(printBuffer, 32, formatString);
			OutputDebugStringA(printBuffer);
			break;
		}
		case 2:
		{
			sprintf_s(printBuffer, 32, formatString, opCode[1]);
			OutputDebugStringA(printBuffer);
			break;
		}
		case 3:
		{
			sprintf_s(printBuffer, 32, formatString, opCode[2], opCode[1]);
			OutputDebugStringA(printBuffer);
			break;
		}
	}
}

internal_func u16 PrintDisassembly(char *tempPrint, unsigned char *opCode)
{
    u8 opBytes = 1;
	switch(*opCode)
	{
		// 0x0 ///////////////////////////////////////////////////////////////////////////
		
		case 0x01:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "LXI\tB,%02x%02x\n", opCode);
			break;
		}
		
		case 0x02:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "STAX\tB\n", opCode);
			break;
		}
		
		case 0x03:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INX\tB\n", opCode);
			break;
		}
		
		case 0x04:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INR\tB\n", opCode);
			break;
		}
		
		case 0x05:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCR\tB\n", opCode);
			break;
		}
		
		case 0x06:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "MVI\tB,%02x\n", opCode);
			break;
		}
		
		case 0x07:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RLC\n", opCode);
			break;
		}
		
		case 0x09:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DAD\tB\n", opCode);
			break;
		}
		
		case 0x0a:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "LDAX\tB\n", opCode);
			break;
		}
		
		case 0x0b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCX\tB\n", opCode);
			break;
		}
		
		case 0x0c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INR\tC\n", opCode);
			break;
		}
		
		case 0x0d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCR\tC\n", opCode);
			break;
		}
		
		case 0x0e:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "MVI\tC,%02x\n", opCode);
			break;
		}
		
		case 0x0f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RRC\n", opCode);
			break;
		}
		
		// 0x1 ///////////////////////////////////////////////////////////////////////////
		
		case 0x11:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "LXI\tD,%02x%02x\n", opCode);
			break;
		}
		
		case 0x12:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "STAX\tD\n", opCode);
			break;
		}
		
		case 0x13:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INX\tD\n", opCode);
			break;
		}
		
		case 0x14:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INR\tD\n", opCode);
			break;
		}
		
		case 0x15:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCR\tD\n", opCode);
			break;
		}
		
		case 0x16:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "MVI\tD,%02x\n", opCode);
			break;
		}
		
		case 0x17:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RAL\n", opCode);
			break;
		}
		
		case 0x19:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DAD\tD\n", opCode);
			break;
		}
		
		case 0x1a:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "LDAX\tD\n", opCode);
			break;
		}
		
		case 0x1b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCX\tD\n", opCode);
			break;
		}
		
		case 0x1c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INR\tE\n", opCode);
			break;
		}
		
		case 0x1d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCR\tE\n", opCode);
			break;
		}
		
		case 0x1e:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "MVI\tE,%02x\n", opCode);
			break;
		}
		
		case 0x1f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RAR\n", opCode);
			break;
		}
		
		// 0x2 ///////////////////////////////////////////////////////////////////////////
		
		case 0x21:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "LXI\tH,%02x%02x\n", opCode);
			break;
		}
		
		case 0x22:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "SHLD\t%02x%02x\n", opCode);
			break;
		}
		
		case 0x23:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INX\tH\n", opCode);
			break;
		}
		
		case 0x24:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INR\tH\n", opCode);
			break;
		}
		
		case 0x25:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCR\tH\n", opCode);
			break;
		}
		
		case 0x26:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "MVI\tH,%02x\n", opCode);
			break;
		}
		
		case 0x27:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DAA\n", opCode);
			break;
		}
		
		case 0x29:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DAD\tH\n", opCode);
			break;
		}
		
		case 0x2a:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "LHLD\t%02x%02x\n", opCode);
			break;
		}
		
		case 0x2b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCX\tH\n", opCode);
			break;
		}
		
		case 0x2c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INR\tL\n", opCode);
			break;
		}
		
		case 0x2d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCR\tL\n", opCode);
			break;
		}
		
		case 0x2e:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "MVI\tL,%02x\n", opCode);
			break;
		}
		
		case 0x2f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMA\n", opCode);
			break;
		}
		
		// 0x3 ///////////////////////////////////////////////////////////////////////////
		
		case 0x31:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "LXI\tSP,%02x%02x\n", opCode);
			break;
		}
		
		case 0x32:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "STA\t%02x%02x\n", opCode);
			break;
		}
		
		case 0x33:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INX\tSP\n", opCode);
			break;
		}
		
		case 0x34:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INR\tM\n", opCode);
			break;
		}
		
		case 0x35:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCR\tM\n", opCode);
			break;
		}
		
		case 0x36:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "MVI\tM,%02x\n", opCode);
			break;
		}
		
		case 0x37:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "STC\n", opCode);
			break;
		}
		
		case 0x39:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DAD\tSP\n", opCode);
			break;
		}
		
		case 0x3a:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "LDA\t%02x%02x\n", opCode);
			break;
		}
		
		case 0x3b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCX\tSP\n", opCode);
			break;
		}
		
		case 0x3c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "INR\tA\n", opCode);
			break;
		}
		
		case 0x3d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DCR\tA\n", opCode);
			break;
		}
		
		case 0x3e:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "MVI\tA,%02x\n", opCode);
			break;
		}
		
		case 0x3f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMC\n", opCode);
			break;
		}
		
		// 0x4 ///////////////////////////////////////////////////////////////////////////
		
		case 0x40:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tB,B\n", opCode);
			break;
		}
		
		case 0x41:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tB,C\n", opCode);
			break;
		}
		
		case 0x42:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tB,D\n", opCode);
			break;
		}
		
		case 0x43:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tB,E\n", opCode);
			break;
		}
		
		case 0x44:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tB,H\n", opCode);
			break;
		}
		
		case 0x45:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tB,L\n", opCode);
			break;
		}
		
		case 0x46:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tB,M\n", opCode);
			break;
		}
		
		case 0x47:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tB,A\n", opCode);
			break;
		}
		
		case 0x48:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tC,B\n", opCode);
			break;
		}
		
		case 0x49:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tC,C\n", opCode);
			break;
		}
		
		case 0x4a:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tC,D\n", opCode);
			break;
		}
		
		case 0x4b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tC,E\n", opCode);
			break;
		}
		
		case 0x4c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tC,H\n", opCode);
			break;
		}
		
		case 0x4d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tC,L\n", opCode);
			break;
		}
		
		case 0x4e:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tC,M\n", opCode);
			break;
		}
		
		case 0x4f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tC,A\n", opCode);
			break;
		}
		
		// 0x5 ///////////////////////////////////////////////////////////////////////////
		
		case 0x50:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tD,B\n", opCode);
			break;
		}
		
		case 0x51:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tD,C\n", opCode);
			break;
		}
		
		case 0x52:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tD,D\n", opCode);
			break;
		}
		
		case 0x53:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tD,E\n", opCode);
			break;
		}
		
		case 0x54:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tD,H\n", opCode);
			break;
		}
		
		case 0x55:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tD,L\n", opCode);
			break;
		}
		
		case 0x56:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tD,M\n", opCode);
			break;
		}
		
		case 0x57:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tD,A\n", opCode);
			break;
		}
		
		case 0x58:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tE,B\n", opCode);
			break;
		}
		
		case 0x59:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tE,C\n", opCode);
			break;
		}
		
		case 0x5a:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tE,D\n", opCode);
			break;
		}
		
		case 0x5b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tE,E\n", opCode);
			break;
		}
		
		case 0x5c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tE,H\n", opCode);
			break;
		}
		
		case 0x5d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tE,L\n", opCode);
			break;
		}
		
		case 0x5e:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tE,M\n", opCode);
			break;
		}
		
		case 0x5f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tE,A\n", opCode);
			break;
		}
		
		// 0x6 ///////////////////////////////////////////////////////////////////////////
		
		case 0x60:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tH,B\n", opCode);
			break;
		}
		
		case 0x61:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tH,C\n", opCode);
			break;
		}
		
		case 0x62:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tH,D\n", opCode);
			break;
		}
		
		case 0x63:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tH,E\n", opCode);
			break;
		}
		
		case 0x64:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tH,H\n", opCode);
			break;
		}
		
		case 0x65:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tH,L\n", opCode);
			break;
		}
		
		case 0x66:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tH,M\n", opCode);
			break;
		}
		
		case 0x67:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tH,A\n", opCode);
			break;
		}
		
		case 0x68:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tL,B\n", opCode);
			break;
		}
		
		case 0x69:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tL,C\n", opCode);
			break;
		}
		
		case 0x6a:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tL,D\n", opCode);
			break;
		}
		
		case 0x6b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tL,E\n", opCode);
			break;
		}
		
		case 0x6c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tL,H\n", opCode);
			break;
		}
		
		case 0x6d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tL,L\n", opCode);
			break;
		}
		
		case 0x6e:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tL,M\n", opCode);
			break;
		}
		
		case 0x6f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tL,A\n", opCode);
			break;
		}
		
		// 0x7 ///////////////////////////////////////////////////////////////////////////
		
		case 0x70:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tM,B\n", opCode);
			break;
		}
		
		case 0x71:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tM,C\n", opCode);
			break;
		}
		
		case 0x72:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tM,D\n", opCode);
			break;
		}
		
		case 0x73:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tM,E\n", opCode);
			break;
		}
		
		case 0x74:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tM,H\n", opCode);
			break;
		}
		
		case 0x75:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tM,L\n", opCode);
			break;
		}
		
		case 0x76:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "HLT\n", opCode);
			break;
		}
		
		case 0x77:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tM,A\n", opCode);
			break;
		}
		
		case 0x78:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tA,B\n", opCode);
			break;
		}
		
		case 0x79:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tA,C\n", opCode);
			break;
		}
		
		case 0x7a:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tA,D\n", opCode);
			break;
		}
		
		case 0x7b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tA,E\n", opCode);
			break;
		}
		
		case 0x7c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tA,H\n", opCode);
			break;
		}
		
		case 0x7d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tA,L\n", opCode);
			break;
		}
		
		case 0x7e:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tA,M\n", opCode);
			break;
		}
		
		case 0x7f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "MOV\tA,A\n", opCode);
			break;
		}
		
		// 0x8 ///////////////////////////////////////////////////////////////////////////
		
		case 0x80:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADD\tB\n", opCode);
			break;
		}
		
		case 0x81:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADD\tC\n", opCode);
			break;
		}
		
		case 0x82:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADD\tD\n", opCode);
			break;
		}
		
		case 0x83:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADD\tE\n", opCode);
			break;
		}
		
		case 0x84:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADD\tH\n", opCode);
			break;
		}
		
		case 0x85:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADD\tL\n", opCode);
			break;
		}
		
		case 0x86:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADD\tM\n", opCode);
			break;
		}
		
		case 0x87:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADD\tA\n", opCode);
			break;
		}
		
		case 0x88:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADC\tB\n", opCode);
			break;
		}
		
		case 0x89:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADC\tC\n", opCode);
			break;
		}
		
		case 0x8a:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADC\tD\n", opCode);
			break;
		}
		
		case 0x8b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADC\tE\n", opCode);
			break;
		}
		
		case 0x8c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADC\tH\n", opCode);
			break;
		}
		
		case 0x8d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADC\tL\n", opCode);
			break;
		}
		
		case 0x8e:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADC\tM\n", opCode);
			break;
		}
		
		case 0x8f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADC\tA\n", opCode);
			break;
		}
		
		// 0x9 ///////////////////////////////////////////////////////////////////////////
		
		case 0x90:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUB\tB\n", opCode);
			break;
		}
		
		case 0x91:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUB\tC\n", opCode);
			break;
		}
		
		case 0x92:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUB\tD\n", opCode);
			break;
		}
		
		case 0x93:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUB\tE\n", opCode);
			break;
		}
		
		case 0x94:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUB\tH\n", opCode);
			break;
		}
		
		case 0x95:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUB\tL\n", opCode);
			break;
		}
		
		case 0x96:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUB\tM\n", opCode);
			break;
		}
		
		case 0x97:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUB\tA\n", opCode);
			break;
		}
		
		case 0x98:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBB\tB\n", opCode);
			break;
		}
		
		case 0x99:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBB\tC\n", opCode);
			break;
		}
		
		case 0x9a:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBB\tD\n", opCode);
			break;
		}
		
		case 0x9b:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBB\tE\n", opCode);
			break;
		}
		
		case 0x9c:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBB\tH\n", opCode);
			break;
		}
		
		case 0x9d:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBB\tL\n", opCode);
			break;
		}
		
		case 0x9e:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBB\tM\n", opCode);
			break;
		}
		
		case 0x9f:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBB\tA\n", opCode);
			break;
		}
		
		// 0xa ///////////////////////////////////////////////////////////////////////////
		
		case 0xa0:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANA\tB\n", opCode);
			break;
		}
		
		case 0xa1:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANA\tC\n", opCode);
			break;
		}
		
		case 0xa2:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANA\tD\n", opCode);
			break;
		}
		
		case 0xa3:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANA\tE\n", opCode);
			break;
		}
		
		case 0xa4:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANA\tH\n", opCode);
			break;
		}
		
		case 0xa5:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANA\tL\n", opCode);
			break;
		}
		
		case 0xa6:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANA\tM\n", opCode);
			break;
		}
		
		case 0xa7:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANA\tA\n", opCode);
			break;
		}
		
		case 0xa8:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRA\tB\n", opCode);
			break;
		}
		
		case 0xa9:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRA\tC\n", opCode);
			break;
		}
		
		case 0xaa:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRA\tD\n", opCode);
			break;
		}
		
		case 0xab:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRA\tE\n", opCode);
			break;
		}
		
		case 0xac:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRA\tH\n", opCode);
			break;
		}
		
		case 0xad:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRA\tL\n", opCode);
			break;
		}
		
		case 0xae:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRA\tM\n", opCode);
			break;
		}
		
		case 0xaf:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRA\tA\n", opCode);
			break;
		}
		
		// 0xb ///////////////////////////////////////////////////////////////////////////
		
		case 0xb0:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORA\tB\n", opCode);
			break;
		}
		
		case 0xb1:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORA\tC\n", opCode);
			break;
		}
		
		case 0xb2:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORA\tD\n", opCode);
			break;
		}
		
		case 0xb3:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORA\tE\n", opCode);
			break;
		}
		
		case 0xb4:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORA\tH\n", opCode);
			break;
		}
		
		case 0xb5:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORA\tL\n", opCode);
			break;
		}
		
		case 0xb6:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORA\tM\n", opCode);
			break;
		}
		
		case 0xb7:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORA\tA\n", opCode);
			break;
		}
		
		case 0xb8:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMP\tB\n", opCode);
			break;
		}
		
		case 0xb9:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMP\tC\n", opCode);
			break;
		}
		
		case 0xba:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMP\tD\n", opCode);
			break;
		}
		
		case 0xbb:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMP\tE\n", opCode);
			break;
		}
		
		case 0xbc:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMP\tH\n", opCode);
			break;
		}
		
		case 0xbd:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMP\tL\n", opCode);
			break;
		}
		
		case 0xbe:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMP\tM\n", opCode);
			break;
		}
		
		case 0xbf:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "CMP\tA\n", opCode);
			break;
		}
		
		// 0xc ///////////////////////////////////////////////////////////////////////////
		
		case 0xc0:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RNZ\n", opCode);
			break;
		}
		
		case 0xc1:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "POP\tB\n", opCode);
			break;
		}
		
		case 0xc2:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JNZ\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xc3:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JMP\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xc4:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CNZ\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xc5:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "PUSH\tB\n", opCode);
			break;
		}
		
		case 0xc6:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "ADI\t%02x\n", opCode);
			break;
		}
		
		case 0xc7:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RST\t0\n", opCode);
			break;
		}
		
		case 0xc8:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RZ\n", opCode);
			break;
		}
		
		case 0xc9:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RET\n", opCode);
			break;
		}
		
		case 0xca:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JZ\t\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xcb:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JMP\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xcc:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CZ\t\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xcd:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CALL\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xce:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "ACI\t%02x\n", opCode);
			break;
		}
		
		case 0xcf:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RST\t1\n", opCode);
			break;
		}
		
		// 0xd ///////////////////////////////////////////////////////////////////////////
		
		case 0xd0:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RNC\n", opCode);
			break;
		}
		
		case 0xd1:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "POP\tD\n", opCode);
			break;
		}
		
		case 0xd2:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JNC\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xd3:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "OUT\t%02x\n", opCode);
			break;
		}
		
		case 0xd4:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CNC\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xd5:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "PUSH\tD\n", opCode);
			break;
		}
		
		case 0xd6:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "SUI\t%02x\n", opCode);
			break;
		}
		
		case 0xd7:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RST\t2\n", opCode);
			break;
		}
		
		case 0xd8:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RC\n", opCode);
			break;
		}
		
		case 0xd9:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RET\n", opCode);
			break;
		}
		
		case 0xda:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JC\t\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xdb:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "IN\t\t%02x\n", opCode);
			break;
		}
		
		case 0xdc:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CC\t\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xdd:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CALL\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xde:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "SBI\t%02x\n", opCode);
			break;
		}
		
		case 0xdf:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RST\t3\n", opCode);
			break;
		}
		
		// 0xe ///////////////////////////////////////////////////////////////////////////
		
		case 0xe0:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RPO\n", opCode);
			break;
		}
		
		case 0xe1:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "POP\tH\n", opCode);
			break;
		}
		
		case 0xe2:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JPO\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xe3:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XTHL\n", opCode);
			break;
		}
		
		case 0xe4:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CPO\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xe5:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "PUSH\tH\n", opCode);
			break;
		}
		
		case 0xe6:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "ANI\t%02x\n", opCode);
			break;
		}
		
		case 0xe7:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RST\t4\n", opCode);
			break;
		}
		
		case 0xe8:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RPE\n", opCode);
			break;
		}
		
		case 0xe9:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "PCHL\n", opCode);
			break;
		}
		
		case 0xea:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JPE\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xeb:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "XCHG\n", opCode);
			break;
		}
		
		case 0xec:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CPE\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xed:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CALL\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xee:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "XRI\t%02x\n", opCode);
			break;
		}
		
		case 0xef:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RST\t5\n", opCode);
			break;
		}
		
		// 0xf ///////////////////////////////////////////////////////////////////////////
		
		case 0xf0:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RP\n", opCode);
			break;
		}
		
		case 0xf1:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "POP\tPSW\n", opCode);
			break;
		}
		
		case 0xf2:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JP\t\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xf3:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "DI\n", opCode);
			break;
		}
		
		case 0xf4:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CP\t\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xf5:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "PUSH\tPSW\n", opCode);
			break;
		}
		
		case 0xf6:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "ORI\t%02x\n", opCode);
			break;
		}
		
		case 0xf7:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RST\t6\n", opCode);
			break;
		}
		
		case 0xf8:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RM\n", opCode);
			break;
		}
		
		case 0xf9:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "SPHL\n", opCode);
			break;
		}
		
		case 0xfa:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "JM\t\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xfb:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "EI\n", opCode);
			break;
		}
		
		case 0xfc:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CM\t\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xfd:
		{
			opBytes = 3;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CALL\t%02x%02x\n", opCode);
			break;
		}
		
		case 0xfe:
		{
			opBytes = 2;
			DebugPrintDisassembledROM(tempPrint, opBytes, "CPI\t%02x\n", opCode);
			break;
		}
		
		case 0xff:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "RST\t7\n", opCode);
			break;
		}
		
		default:
		{
			DebugPrintDisassembledROM(tempPrint, opBytes, "NOP\n", opCode);
			break;
		}
	}
	
	return opBytes;
}
#endif


