#pragma once

#include <malloc.h>
#include "types.h"

// Convert a color in 24-bit RGB888 format to 16-bit RGB565 format.
WORD rgb888_to_rgb565(const BYTE* color) {
	return ((color[0] >> 3) << 11) | ((color[1] >> 2) << 5) | (color[2] >> 3);
}

// Convert a color in 16-bit RGB565 format to 24-bit RGB888 format.
BYTE *rgb565_to_rgb888(const WORD *color) {
	BYTE *result = (BYTE*)malloc(3);
	result[0] = (BYTE)((*color >> 11) & 0x1F) << 3;
	result[1] = (BYTE)((*color >> 5) & 0x3F) << 2;
	result[2] = (BYTE)(*color & 0x1F) << 3;

	return result;
}

// Convert RGB image to RGBA image (Alpha = 255)
void rgb_to_rgba_image(const BYTE* inBuff, BYTE* outBuff, unsigned long pixelCount) {
	for (unsigned long i = 0; i < pixelCount; i++) {
		memcpy(outBuff, inBuff, 3);
		outBuff[3] = 255;
		outBuff += 4;
		inBuff += 3;
	}
}