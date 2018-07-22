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

struct CPUFlags
{
	u8 z : 1;
	u8 s : 1;
	u8 unused1 : 1;
	u8 a : 1;
	u8 unused2 : 1;
	u8 p : 1;
	u8 unused3 : 1;
	u8 c : 1;
};

struct CPUState
{
	// PSW
	// B
	// D
	// H
	
	u8 regA;
	u8 regB;
	u8 regD;
	u8 regH;
	
	CPUFlags regF;
	u8 regC;
	u8 regE;
	u8 regL;
	
	u8 *memory;
	u8 intEnable;
	u16 stackPointer;
	u16 programCounter;
};


