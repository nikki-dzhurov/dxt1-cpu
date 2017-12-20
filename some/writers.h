#pragma once

#include "types.h"

void writeByte(BYTE*& dest, BYTE b) {
	dest[0] = b;
	dest += 1;
}

void writeWord(BYTE*& dest, WORD s) {
	dest[0] = (s >> 0) & 255;
	dest[1] = (s >> 8) & 255;
	dest += 2;
}

void writeDoubleWord(BYTE*& dest, DWORD i) {
	dest[0] = (i >> 0) & 255;
	dest[1] = (i >> 8) & 255;
	dest[2] = (i >> 16) & 255;
	dest[3] = (i >> 24) & 255;
	dest += 4;
}

void setRGBAPixel(BYTE *dst, BYTE r, BYTE g, BYTE b, BYTE a) {
	dst[0] = r;
	dst[1] = g;
	dst[2] = b;
	dst[3] = a;
}