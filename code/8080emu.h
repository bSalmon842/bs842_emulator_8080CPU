/*
Project: Intel 8080 CPU Emulator
File: win32_8080emu.h
Author: Brock Salmon
Notice: (C) Copyright 2018 by Brock Salmon. All Rights Reserved.
*/

// Static Definitions
#define internal_func static
#define local_persist static
#define global_var static

// Typedefs
#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#if EMU8080_SLOW
#define ASSERT(expr) if(!(expr)) {*(int *)0 = 0;}
#else
#define ASSERT(expr)
#endif

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)

enum class Port1MachineKeys
{
	COIN,
	P2START,
	P1START,
	UNUSED1,
	P1SHOOT,
	P1LEFT,
	P1RIGHT,
	UNUSED2
};

enum class Port2MachineKeys
{
	DIPSWITCH1,
	DIPSWITCH2,
	TILT,
	DIPSWITCHBONUS,
	P2SHOOT,
	P2LEFT,
	P2RIGHT,
	DIPSWITCHCOIN
};

struct CPUFlags
{
	u8 s : 1;
	u8 z : 1;
	u8 unused1 : 1;
	u8 a : 1;
	u8 unused2 : 1;
	u8 p : 1;
	u8 unused3 : 1;
	u8 c : 1;
};

struct CPUState
{
	u8 regA;
	CPUFlags regF;
	
	u8 regB;
	u8 regC;
	
	u8 regD;
	u8 regE;
	
	u8 regH;
	u8 regL;
	
	u8 *memory;
	u8 enableInterrupt;
	u16 stackPointer;
	u16 programCounter;
	
	u8 shift0;
	u8 shift1;
	u8 shiftOffset;
	
	u8 inputPort1;
	u8 inputPort2;
};

struct BackBuffer
{
	// NOTE[bSalmon]: 32-bit wide, Mem Order BB GG RR xx
	void *memory;
	s32 width;
	s32 height;
	s32 pitch;
	s32 bytesPerPixel;
};

// NOTE(bSalmon): From Emulator 101, Array of cycles values for the opcodes, used as: cycleArray[opCode], might change to each instruction individually adding the cycles to currentCycles instead
global_var u8 cyclesArray[] = {
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4,
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4,
	4, 10, 16, 5, 5, 5, 7, 4, 4, 10, 16, 5, 5, 5, 7, 4,
	4, 10, 13, 5, 10, 10, 10, 4, 4, 10, 13, 5, 5, 5, 7, 4,
	
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,
	
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11,
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, 
	11, 10, 10, 18, 17, 11, 7, 11, 11, 5, 10, 5, 17, 17, 7, 11, 
	11, 10, 10, 4, 17, 11, 7, 11, 11, 5, 10, 4, 17, 17, 7, 11, 
};
