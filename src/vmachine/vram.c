/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vram.h"

t_ram vram;

void vramAlloc(t_nubitcc newsize)
{
	if (newsize) {
		vram.size = newsize;
		if (vram.base) free((void *)vram.base);
		vram.base = (t_vaddrcc)malloc(vram.size);
		memset((void *)vram.base, 0x00, vram.size);
	}
}
void vramInit()
{
	memset(&vram, 0x00, sizeof(t_ram));
	vramAlloc(1 << 22);
}
void vramReset()
{
	memset((void *)vram.base, 0x00, vram.size);
}
void vramRefresh() {}
void vramFinal()
{
	if (vram.base) free((void *)(vram.base));
}
