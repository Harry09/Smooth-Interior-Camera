/*
*********************************************************************
* File			: MemMgr.cpp
* Project		: Smooth Interior Camera
* Developers	: Piotr Krupa (piotrkrupa06@gmail.com)
* License		: MIT License
*********************************************************************
*/

#include "MemMgr.h"

#include <Windows.h>

/*
	push rax
	mov rax addr   ; @addr - address to original
	xchg qword ptr ss:[rsp], rax
	ret
*/
BYTE g_jmpHook[] = { 0x50, 0x48, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x87, 0x04, 0x24, 0xC3 };

void MemMgr::UnprotectMemory(uintptr_t addr, size_t size)
{
	DWORD temp;
	VirtualProtect((void*)addr, size, PAGE_EXECUTE_READWRITE, &temp);
}

void MemMgr::LongJmpHook(uintptr_t addr, uintptr_t addrTo)
{
	UnprotectMemory(addr, sizeof(g_jmpHook));

	memcpy((unsigned char*)addr, g_jmpHook, sizeof(g_jmpHook));
	*(uintptr_t*)&((unsigned char*)addr)[3] = addrTo;
}