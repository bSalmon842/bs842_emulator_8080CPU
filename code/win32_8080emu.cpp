/*
Project: Intel 8080 CPU Emulator
File: win32_8080emu.cpp
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

/*
NOTE(bSalmon):

8080_INTERNAL:
0 - Public Build
1 - Dev Build

8080_SLOW:
 0 - Assert Code Disabled
 1 - Assert debugging code enabled
*/

/*
EMULATOR MEMORY LAYOUT:
0000 - 1fff: ROM Memory
2000 - 23ff: Work RAM
2400 - 3fff: Video RAM
*/

// TODO LIST
// TODO(bSalmon): Sound (Put off for the time being as it would be
// difficult to implement without external libraries being used
// TODO(bSalmon): Rework Code to make Platform Specific and Non-Specific Functions obvious

#include <Windows.h>
#include "8080emu.cpp"

#if EMU8080_INTERNAL
#include <stdio.h>
#include "8080emu_disassemble.cpp"
#endif

struct Win32_BackBuffer
{
	// NOTE[bSalmon]: 32-bit wide, Mem Order BB GG RR xx
	BITMAPINFO info;
	void *memory;
	s32 width;
	s32 height;
	s32 pitch;
	s32 bytesPerPixel;
};

struct Win32_WindowDimensions
{
	s32 width;
	s32 height;
};

struct Win32_Menus
{
	HMENU emulatorOptions;
	HMENU settings;
};

global_var b32 globalRunning;
global_var Win32_BackBuffer globalBackBuffer = {};

// Return a struct containing the height and width of the window bitmap
internal_func Win32_WindowDimensions Win32_GetWindowDimensions(HWND window)
{
	Win32_WindowDimensions result;
	
	RECT clientRect;
	GetClientRect(window, &clientRect);
	result.width = clientRect.right - clientRect.left;
	result.height = clientRect.bottom - clientRect.top;
	
	return result;
}

// Resize Device Independent Bitmap Section
internal_func void Win32_ResizeDIBSection(Win32_BackBuffer *buffer, CPUState *cpuState, s32 width, s32 height)
{
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}
	
	buffer->width = width;
	buffer->height = height;
	buffer->bytesPerPixel = 4;
	
	// NOTE[bSalmon]: If biHeight is negative, bitmap is top-down
	// (first 4 bytes are the top left pixel)
	buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
	buffer->info.bmiHeader.biWidth = buffer->width;
	buffer->info.bmiHeader.biHeight = -buffer->height;
	buffer->info.bmiHeader.biPlanes = 1;
	buffer->info.bmiHeader.biBitCount = 32;
	buffer->info.bmiHeader.biCompression = BI_RGB;
	
	s32 bitmapMemorySize = (buffer->width * buffer->height) * buffer->bytesPerPixel;
	buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	buffer->pitch = width * buffer->bytesPerPixel;
}

// Present the Back Buffer to the screen
internal_func void Win32_PresentBuffer(HDC deviceContext, s32 windowWidth, s32 windowHeight, Win32_BackBuffer *buffer)
{
	StretchDIBits(deviceContext,
				  0, 0, windowWidth, windowHeight,	/// DEST
				  0, 0, buffer->width, buffer->height,	/// SRC
				  buffer->memory,
				  &buffer->info,
				  DIB_RGB_COLORS,
				  SRCCOPY);
}

internal_func void Win32_ResetEmulator(CPUState *cpuState, MachineState *machine)
{
	// NOTE(bSalmon): This function is for clearing the memory of and resetting the emulator,
	// however, the romSize and romFilename are left alone, as this function is used
	// on Loading a new ROM
	
	// Reset Emulator Memory
	if (cpuState->memory)
	{
		VirtualFree(cpuState->memory, 0, MEM_RELEASE);
	}
	
	cpuState->regA = 0x00;
	cpuState->regF = {};
	
	cpuState->regB = 0x00;
	cpuState->regC = 0x00;
	
	cpuState->regD = 0x00;
	cpuState->regE = 0x00;
	
	cpuState->regH = 0x00;
	cpuState->regL = 0x00;
	
	cpuState->memory = 0;
	cpuState->enableInterrupt = false;
	cpuState->stackPointer = 0x0000;
	cpuState->programCounter = 0x0000;
	
	machine->shift0 = 0x00;
	machine->shift1 = 0x00;
	machine->shiftOffset = 0x00;
	
	machine->inputPort1 = 0x00;
	machine->inputPort2 = 0x00;
}

internal_func void Win32_LoadROM(CPUState *cpuState, MachineState *machine)
{
	Win32_ResetEmulator(cpuState, machine);
	cpuState->memory = (u8 *)VirtualAlloc(0, MEGABYTES(1), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	HANDLE romHandle =  CreateFileA(machine->romFilename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (romHandle != INVALID_HANDLE_VALUE)
	{
		if (cpuState->memory)
		{
			DWORD bytesRead;
			ReadFile(romHandle, cpuState->memory, machine->romSize, &bytesRead, 0);
		}
		
		CloseHandle(romHandle);
	}
}

internal_func void Win32_HandleMenuCommands(CPUState *cpuState, MachineState *machine, Win32_Menus menus, HWND window, WPARAM wParam, LPARAM lParam)
{
	// NOTE(bSalmon): Emulator Options and Settings currently only have one item so there is no need for nested if statements
	HMENU selectedMenu = (HMENU)lParam;
	s32 itemPos = wParam;
	if (selectedMenu == menus.emulatorOptions)
	{
		OPENFILENAMEA openFileNameInfo = {};
		openFileNameInfo.lStructSize = sizeof(OPENFILENAMEA);
		openFileNameInfo.hwndOwner = window;
		openFileNameInfo.lpstrFilter = "All Files\0*.*\0\0";
		openFileNameInfo.lpstrFile = machine->romFilename;
		openFileNameInfo.nMaxFile = 256;
		openFileNameInfo.lpstrTitle = "Open 8080 ROM File";
		openFileNameInfo.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileNameA(&openFileNameInfo))
		{
			Win32_LoadROM(cpuState, machine);
		}
	}
	else if (selectedMenu == menus.settings)
	{
		MENUITEMINFOA menuItemInfo = {};
		menuItemInfo.cbSize = sizeof(MENUITEMINFOA);
		if (machine->enableColour)
		{
			menuItemInfo.fMask = MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_STATE | MIIM_STRING ;
			menuItemInfo.fType = MFT_STRING;
			menuItemInfo.fState = MFS_UNCHECKED;
			menuItemInfo.dwTypeData = "Enable Colour";
			machine->enableColour = false;
		}
		else
		{
			menuItemInfo.fMask = MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_STATE | MIIM_STRING ;
			menuItemInfo.fType = MFT_STRING;
			menuItemInfo.fState = MFS_CHECKED;
			menuItemInfo.dwTypeData = "Enable Colour";
			machine->enableColour = true;
		}
		SetMenuItemInfo(selectedMenu, itemPos, TRUE, &menuItemInfo);
	}
}

internal_func void Win32_MaintainWindowAspectRatio(HWND window, WPARAM wParam, LPARAM lParam)
{
	RECT *windowRect = (RECT *)lParam;
	f32 aspectRatio = 7.0f/8.0f;
	s32 width = windowRect->right - windowRect->left;
	s32 height = windowRect->bottom - windowRect->top;
	
	if (wParam == WMSZ_LEFT || wParam == WMSZ_RIGHT)
	{
		windowRect->bottom = (width / aspectRatio) + windowRect->top;
	}
	else if (wParam == WMSZ_TOP || wParam == WMSZ_BOTTOM)
	{
		windowRect->right = (height * aspectRatio) + windowRect->left;
	}
	else if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT)
	{
		if (((float)width / (float)height) > aspectRatio)
		{
			width = (s32)((float)height * aspectRatio);
		}
		else
		{
			height = (s32)((float)width / aspectRatio);
		}
		
		if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
		{
			windowRect->top = windowRect->bottom - height;
		}
		else
		{
			windowRect->bottom = windowRect->top + height;
		}
		
		if (wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
		{
			windowRect->left = windowRect->right - width;
		}
		else
		{
			windowRect->right = windowRect->left + width;
		}
	}
	
	s32 newWidth = windowRect->right - windowRect->left;
	s32 newHeight = windowRect->bottom - windowRect->top;
	
	SetWindowPos(window, 0, 
				 windowRect->left, windowRect->top, 
				 newWidth, newHeight, 0);
}

LRESULT CALLBACK Win32_WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	
	switch (msg)
	{
		case WM_CLOSE:
		{
			globalRunning = false;
			break;
		}
		
		case WM_SIZING:
		{
			Win32_MaintainWindowAspectRatio(window, wParam, lParam);
			break;
		}
		
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(window, &paint);
			
			Win32_WindowDimensions windowDim = Win32_GetWindowDimensions(window);
			Win32_PresentBuffer(deviceContext, windowDim.width, windowDim.height, &globalBackBuffer);
			EndPaint(window, &paint);
			break;
		}
		
		default:
		{
			result = DefWindowProcA(window, msg, wParam, lParam);
			break;
		}
	}
	
	return result;
}

internal_func void Win32_HandleKeyDown(CPUState *cpuState, MachineState *machine, MSG message)
{
	u32 vkCode = (u32)message.wParam;
	b32 keyWasDown = ((message.lParam & (1 << 30)) != 0);
	b32 keyIsDown = ((message.lParam & (1 << 31)) == 0);
	if (keyWasDown != keyIsDown)
	{
		// Port 1
		if (vkCode == 'A')
		{
			ProcessMachineKeyDown(&machine->inputPort1, (u8)Port1MachineKeys::P1LEFT);
		}
		else if (vkCode == 'D')
		{
			ProcessMachineKeyDown(&machine->inputPort1, (u8)Port1MachineKeys::P1RIGHT);
		}
		else if (vkCode == VK_SPACE)
		{
			ProcessMachineKeyDown(&machine->inputPort1, (u8)Port1MachineKeys::P1SHOOT);
		}
		else if (vkCode == 'C')
		{
			ProcessMachineKeyDown(&machine->inputPort1, (u8)Port1MachineKeys::COIN);
		}
		else if (vkCode == VK_SHIFT)
		{
			ProcessMachineKeyDown(&machine->inputPort1, (u8)Port1MachineKeys::P1START);
		}
		else if (vkCode == VK_RETURN)
		{
			ProcessMachineKeyDown(&machine->inputPort1, (u8)Port1MachineKeys::P2START);
		}
		
		// Port 2
		else if (vkCode == VK_LEFT)
		{
			ProcessMachineKeyDown(&machine->inputPort2, (u8)Port2MachineKeys::P2LEFT);
		}
		else if (vkCode == VK_RIGHT)
		{
			ProcessMachineKeyDown(&machine->inputPort2, (u8)Port2MachineKeys::P2RIGHT);
		}
		else if (vkCode == VK_UP)
		{
			ProcessMachineKeyDown(&machine->inputPort2, (u8)Port2MachineKeys::P2SHOOT);
		}
		else if (vkCode == '6')
		{
			ProcessMachineKeyDown(&machine->inputPort2, (u8)Port2MachineKeys::DIPSWITCH1);
		}
		else if (vkCode == '7')
		{
			ProcessMachineKeyDown(&machine->inputPort2, (u8)Port2MachineKeys::DIPSWITCH2);
		}
		else if (vkCode == '8')
		{
			ProcessMachineKeyDown(&machine->inputPort2, (u8)Port2MachineKeys::TILT);
		}
		else if (vkCode == '9')
		{
			ProcessMachineKeyDown(&machine->inputPort2, (u8)Port2MachineKeys::DIPSWITCHBONUS);
		}
		else if (vkCode == '0')
		{
			ProcessMachineKeyDown(&machine->inputPort2, (u8)Port2MachineKeys::DIPSWITCHCOIN);
		}
	}
}

internal_func void Win32_HandleKeyUp(CPUState *cpuState, MachineState *machine, MSG message)
{
	u32 vkCode = (u32)message.wParam;
	b32 keyWasDown = ((message.lParam & (1 << 30)) != 0);
	b32 keyIsDown = ((message.lParam & (1 << 31)) == 0);
	if (keyWasDown != keyIsDown)
	{
		// Port 1
		if (vkCode == 'A')
		{
			ProcessMachineKeyUp(&machine->inputPort1, (u8)Port1MachineKeys::P1LEFT);
		}
		else if (vkCode == 'D')
		{
			ProcessMachineKeyUp(&machine->inputPort1, (u8)Port1MachineKeys::P1RIGHT);
		}
		else if (vkCode == VK_SPACE)
		{
			ProcessMachineKeyUp(&machine->inputPort1, (u8)Port1MachineKeys::P1SHOOT);
		}
		else if (vkCode == 'C')
		{
			ProcessMachineKeyUp(&machine->inputPort1, (u8)Port1MachineKeys::COIN);
		}
		else if (vkCode == VK_SHIFT)
		{
			ProcessMachineKeyUp(&machine->inputPort1, (u8)Port1MachineKeys::P1START);
		}
		else if (vkCode == VK_RETURN)
		{
			ProcessMachineKeyUp(&machine->inputPort1, (u8)Port1MachineKeys::P2START);
		}
		
		// Port 2
		else if (vkCode == VK_LEFT)
		{
			ProcessMachineKeyUp(&machine->inputPort2, (u8)Port2MachineKeys::P2LEFT);
		}
		else if (vkCode == VK_RIGHT)
		{
			ProcessMachineKeyUp(&machine->inputPort2, (u8)Port2MachineKeys::P2RIGHT);
		}
		else if (vkCode == VK_UP)
		{
			ProcessMachineKeyUp(&machine->inputPort2, (u8)Port2MachineKeys::P2SHOOT);
		}
		else if (vkCode == '6')
		{
			ProcessMachineKeyUp(&machine->inputPort2, (u8)Port2MachineKeys::DIPSWITCH1);
		}
		else if (vkCode == '7')
		{
			ProcessMachineKeyUp(&machine->inputPort2, (u8)Port2MachineKeys::DIPSWITCH2);
		}
		else if (vkCode == '8')
		{
			ProcessMachineKeyUp(&machine->inputPort2, (u8)Port2MachineKeys::TILT);
		}
		else if (vkCode == '9')
		{
			ProcessMachineKeyUp(&machine->inputPort2, (u8)Port2MachineKeys::DIPSWITCHBONUS);
		}
		else if (vkCode == '0')
		{
			ProcessMachineKeyUp(&machine->inputPort2, (u8)Port2MachineKeys::DIPSWITCHCOIN);
		}
	}
}

s32 CALLBACK WinMain(HINSTANCE currInstance, HINSTANCE prevInstance, LPSTR cmdLine, s32 showCode)
{
	// Set Windows Scheduler Granularity to 1ms for Sleep()
	u32 schedulerGranularity = 1;
	b32 sleepIsGranular = (timeBeginPeriod(schedulerGranularity) == TIMERR_NOERROR);
	
	globalRunning = true;
	
	CPUState cpuState = {};
	MachineState machine = {};
	machine.romSize = 0x2000;
	Win32_ResetEmulator(&cpuState, &machine);
	cpuState.memory = (u8 *)VirtualAlloc(0, MEGABYTES(1), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	WNDCLASSA windowClass = {};
	
	Win32_ResizeDIBSection(&globalBackBuffer, &cpuState, 224, 256);
	
	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Win32_WindowProc;
	windowClass.hInstance = currInstance;
	windowClass.lpszClassName = "8080WindowClass";
	
	f32 monitorRefreshHz = 60.0f;
	f64 targetSecondsPerInterrupt = 1.0f / monitorRefreshHz;
	f64 targetMSPerInterrupt = 1000 * targetSecondsPerInterrupt;
	
	if (RegisterClassA(&windowClass))
	{
		HWND window = CreateWindowExA(0, 
									  windowClass.lpszClassName, "bSalmon842 8080 Emulator", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
									  CW_USEDEFAULT, CW_USEDEFAULT, 
									  224, 256, 
									  0, 0, currInstance, 0);
		
		if (window)
		{
			Win32_Menus menus = {};
			// Menu Programming
			// TODO(bSalmon): Check if a struct of Appended IDs can be used to pass to WindowProc
			HMENU menuBar = CreateMenu();
			
			MENUINFO menuBarInfo = {};
			menuBarInfo.cbSize = sizeof(MENUINFO);
			menuBarInfo.fMask = MIM_APPLYTOSUBMENUS | MIM_STYLE;
			menuBarInfo.dwStyle = MNS_NOTIFYBYPOS;
			
			SetMenuInfo(menuBar, &menuBarInfo);
			
			HMENU emulatorOptions = CreateMenu();
			menus.emulatorOptions = emulatorOptions;
			
			HMENU settings = CreateMenu();
			menus.settings = settings;
			
			AppendMenuA(menuBar, MF_POPUP, (UINT_PTR)emulatorOptions, "Emulator");
			AppendMenuA(menuBar, MF_POPUP, (UINT_PTR)settings, "Settings");
			
			AppendMenuA(emulatorOptions, MF_STRING, 0, "Load ROM");
			
			MENUITEMINFOA enableColourItemInfo = {};
			enableColourItemInfo.cbSize = sizeof(MENUITEMINFOA);
			enableColourItemInfo.fMask = MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_STATE | MIIM_STRING ;
			enableColourItemInfo.fType = MFT_STRING;
			enableColourItemInfo.fState = MFS_UNCHECKED;
			enableColourItemInfo.dwTypeData = "Enable Colour";
			
			BOOL test = InsertMenuItemA(settings, 0, TRUE, &enableColourItemInfo);
			
			SetMenu(window, menuBar);
			
			HDC deviceContext = GetDC(window);
			
			unsigned char *castedMemContents = (unsigned char *)cpuState.memory;
			f64 lastTimer = 0.0f;
			f64 nextInterrupt = 0.0f;
			u8 interruptNum = 1;
			f64 lastFrame = 0.0f;
			
			while (globalRunning)
			{
				MSG message;
				
				while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
				{
					switch(message.message)
					{
						// NOTE(bSalmon): Input is built around the original Space Invaders Hardware
						case WM_SYSKEYDOWN:
						case WM_KEYDOWN:
						{
							Win32_HandleKeyDown(&cpuState, &machine, message);
							break;
						}
						
						case WM_SYSKEYUP:
						case WM_KEYUP:
						{
							Win32_HandleKeyUp(&cpuState, &machine, message);
							break;
						}
						
						case WM_MENUCOMMAND:
						{
							Win32_HandleMenuCommands(&cpuState, &machine, menus, window, message.wParam, message.lParam);
							break;
						}
						
						case WM_QUIT:
						{
							globalRunning = false;
							break;
						}
						
						default:
						{
							TranslateMessage(&message);
							DispatchMessageA(&message);
						}
					}
				}
				
				BackBuffer backBuffer = {};
				backBuffer.memory = globalBackBuffer.memory;
				backBuffer.width = globalBackBuffer.width;
				backBuffer.height = globalBackBuffer.height;
				backBuffer.pitch = globalBackBuffer.pitch;
				backBuffer.bytesPerPixel = globalBackBuffer.bytesPerPixel;
				
				f64 now = GetTickCount();
				u64 cycles = 0;
				
				if (lastTimer == 0.0f)
				{
					lastTimer = now;
					nextInterrupt = lastTimer + targetMSPerInterrupt;
					interruptNum = 1;
				}
				
				if (cpuState.enableInterrupt && (now >= nextInterrupt))
				{
					if (interruptNum == 1)
					{
						Interrupt(&cpuState, 1, &cycles);
						interruptNum = 2;
					}
					else 
					{
						Interrupt(&cpuState, 2, &cycles);
						interruptNum = 1;
					}
					
					RenderVideoMemContents(&backBuffer, &cpuState, machine.enableColour);
					
					nextInterrupt = now + (0.5 * targetMSPerInterrupt);
				}
				
				f64 sinceLast = now - lastTimer;
				s32 cyclesToCatchUp = 2000 * sinceLast;
				
				while (cyclesToCatchUp > cycles)
				{
#if EMU8080_INTERNAL
					local_persist u64 inCount = 0;
					inCount++;
					
					char cpuPrint[128] = {};
					
					sprintf_s(cpuPrint, sizeof(cpuPrint), "\n\n%lld: ", inCount);
					OutputDebugStringA(cpuPrint);
					
					PrintDisassembly(cpuPrint, &castedMemContents[cpuState.programCounter]);
#endif
					
					Emulate(&cpuState, &machine, castedMemContents, &cycles);
					
#if EMU8080_INTERNAL
					sprintf_s(cpuPrint, sizeof(cpuPrint), "\tCPU FLAGS:\nS=%d,Z=%d,A=%d,P=%d,C=%d\n", cpuState.regF.s, cpuState.regF.z, cpuState.regF.a, cpuState.regF.p, cpuState.regF.c);
					OutputDebugStringA(cpuPrint);
					
					u8 psw = BuildPSW(&cpuState);
					
					sprintf_s(cpuPrint, sizeof(cpuPrint), "\tREGISTERS:\nA=%02x, F=%02x, B=%02x, C=%02x, D=%02x, E=%02x, H=%02x, L=%02x, SP=%04x, PC=%04x", cpuState.regA, psw, cpuState.regB, cpuState.regC, cpuState.regD, cpuState.regE, cpuState.regH, cpuState.regL, cpuState.stackPointer, cpuState.programCounter);
					OutputDebugStringA(cpuPrint);
#endif
				}
				
				
				if ((now - lastFrame) >= 16.66667)
				{
					Win32_WindowDimensions windowDim = Win32_GetWindowDimensions(window);
					Win32_PresentBuffer(deviceContext, windowDim.width, windowDim.height, &globalBackBuffer);
					lastFrame = now;
				}
				
				lastTimer = now;
			}
		}
	}
	
	VirtualFree(cpuState.memory, 0, MEM_RELEASE);
	return 0;
}

