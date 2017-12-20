#pragma once

#define ALIGN16(x) __declspec(align(16)) x
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "types.h"
#include "colors.h"
#include "writers.h"

#define INSET_SHIFT 4 // Inset the bounding box with (range >> shift).
#define C565_5_MASK 0xF8 // 0xFF minus last three bits
#define C565_6_MASK 0xFC // 0xFF minus last two bits

void extractBlock(const BYTE* inPtr, int width, BYTE* pixelBlock) {
	for (int j = 0; j < 4; j++) {
		memcpy(&pixelBlock[j * 16], inPtr, 16);
		inPtr += width * 4;
	}
}

void getMinMaxColors(const BYTE* colorBlock, BYTE* minColor, BYTE* maxColor) {
	BYTE inset[3];

	minColor[0] = minColor[1] = minColor[2] = 255;
	maxColor[0] = maxColor[1] = maxColor[2] = 0;

	// Find the bounding box (defined by minimum and maximum color).
	for (int i = 0; i < 16; i++) {
		if (colorBlock[i * 4] < minColor[0]) {
			minColor[0] = colorBlock[i * 4];
		}
		if (colorBlock[i * 4 + 1] < minColor[1]) {
			minColor[1] = colorBlock[i * 4 + 1];
		}
		if (colorBlock[i * 4 + 2] < minColor[2]) {
			minColor[2] = colorBlock[i * 4 + 2];
		}
		if (colorBlock[i * 4] > maxColor[0]) {
			maxColor[0] = colorBlock[i * 4];
		}
		if (colorBlock[i * 4 + 1] > maxColor[1]) {
			maxColor[1] = colorBlock[i * 4 + 1];
		}
		if (colorBlock[i * 4 + 2] > maxColor[2]) {
			maxColor[2] = colorBlock[i * 4 + 2];
		}
	}

	// Inset the bounding box by 1/16 of it's size. (i.e. shift right by 4).
	inset[0] = (maxColor[0] - minColor[0]) >> INSET_SHIFT;
	inset[1] = (maxColor[1] - minColor[1]) >> INSET_SHIFT;
	inset[2] = (maxColor[2] - minColor[2]) >> INSET_SHIFT;

	// Clamp the inset bounding box to 255.
	minColor[0] = (minColor[0] + inset[0] <= 255) ? minColor[0] + inset[0] : 255;
	minColor[1] = (minColor[1] + inset[1] <= 255) ? minColor[1] + inset[1] : 255;
	minColor[2] = (minColor[2] + inset[2] <= 255) ? minColor[2] + inset[2] : 255;

	// Clamp the inset bounding box to 0.
	maxColor[0] = (maxColor[0] >= inset[0]) ? maxColor[0] - inset[0] : 0;
	maxColor[1] = (maxColor[1] >= inset[1]) ? maxColor[1] - inset[1] : 0;
	maxColor[2] = (maxColor[2] >= inset[2]) ? maxColor[2] - inset[2] : 0;
}

void getColorIndices(const BYTE* colorBlock, BYTE*& outBuf, const BYTE* minColor, const BYTE* maxColor, DWORD *colorIndices) {
	WORD colors[4][4];
	*colorIndices = 0;

	colors[0][0] = (maxColor[0] & C565_5_MASK) | (maxColor[0] >> 5);
	colors[0][1] = (maxColor[1] & C565_6_MASK) | (maxColor[1] >> 6);
	colors[0][2] = (maxColor[2] & C565_5_MASK) | (maxColor[2] >> 5);
	colors[1][0] = (minColor[0] & C565_5_MASK) | (minColor[0] >> 5);
	colors[1][1] = (minColor[1] & C565_6_MASK) | (minColor[1] >> 6);
	colors[1][2] = (minColor[2] & C565_5_MASK) | (minColor[2] >> 5);
	colors[2][0] = (2 * colors[0][0] + colors[1][0]) / 3;
	colors[2][1] = (2 * colors[0][1] + colors[1][1]) / 3;
	colors[2][2] = (2 * colors[0][2] + colors[1][2]) / 3;
	colors[3][0] = (colors[0][0] + 2 * colors[1][0]) / 3;
	colors[3][1] = (colors[0][1] + 2 * colors[1][1]) / 3;
	colors[3][2] = (colors[0][2] + 2 * colors[1][2]) / 3;

	for (int i = 15; i >= 0; i--) {
		int r = colorBlock[i * 4];
		int g = colorBlock[i * 4 + 1];
		int b = colorBlock[i * 4 + 2];

		int d0 = abs(colors[0][0] - r) + abs(colors[0][1] - g) + abs(colors[0][2] - b);
		int d1 = abs(colors[1][0] - r) + abs(colors[1][1] - g) + abs(colors[1][2] - b);
		int d2 = abs(colors[2][0] - r) + abs(colors[2][1] - g) + abs(colors[2][2] - b);
		int d3 = abs(colors[3][0] - r) + abs(colors[3][1] - g) + abs(colors[3][2] - b);

		int b0 = d0 > d3;
		int b1 = d1 > d2;
		int b2 = d0 > d2;
		int b3 = d1 > d3;
		int b4 = d2 > d3;

		int x0 = b1 & b2;
		int x1 = b0 & b3;
		int x2 = b0 & b4;

		*colorIndices |= (x2 | ((x0 | x1) << 1)) << (i << 1);
	}
}

void CompressImageDXT1(const BYTE* inBuff, BYTE* outBuff, int width, int height) {
	ALIGN16(BYTE pixelBlock[64]);
	ALIGN16(BYTE minColor[4]);
	ALIGN16(BYTE maxColor[4]);

	DWORD colorIndices;

	for (int j = 0; j < height; j += 4, inBuff += width * 16) {
		for (int i = 0; i < width; i += 4) {
			extractBlock(inBuff + i * 4, width, pixelBlock);
			getMinMaxColors(pixelBlock, minColor, maxColor);
			getColorIndices(pixelBlock, outBuff, minColor, maxColor, &colorIndices);

			// write compressed block
			writeWord(outBuff, rgb888_to_rgb565(maxColor));
			writeWord(outBuff, rgb888_to_rgb565(minColor));
			writeDoubleWord(outBuff, colorIndices);
		}
	}
}

void decompressBlock(int x, int y, int width, BYTE *inBuff, BYTE* out) {
	WORD *col565_0 = reinterpret_cast<WORD *>(inBuff);
	WORD *col565_1 = reinterpret_cast<WORD *>(inBuff + 2);
	DWORD *indices = reinterpret_cast<DWORD *>(inBuff + 4);

	BYTE* color0 = rgb565_to_rgb888(col565_0);
	BYTE* color1 = rgb565_to_rgb888(col565_1);

	BYTE posCode;
	BYTE* pixPtr;

	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			if (x + i < width) {
				posCode = (*indices >> 2 * (4 * j + i)) & 0x3;
				pixPtr = out + ((y + j)*width + (x + i)) * 4;
				if (*col565_0 > *col565_1) {
					switch (posCode) {
					case 0:
						setRGBAPixel(pixPtr, color0[0], color0[1], color0[2], 255);
						break;
					case 1:
						setRGBAPixel(pixPtr, color1[0], color1[1], color1[2], 255);
						break;
					case 2:
						setRGBAPixel(pixPtr, (2 * color0[0] + color1[0]) / 3, (2 * color0[1] + color1[1]) / 3, (2 * color0[2] + color1[2]) / 3, 255);
						break;
					case 3:
						setRGBAPixel(pixPtr, (color0[0] + 2 * color1[0]) / 3, (color0[1] + 2 * color1[1]) / 3, (color0[2] + 2 * color1[2]) / 3, 255);
						break;
					}
				} else {
					switch (posCode) {
					case 0:
						setRGBAPixel(pixPtr, color0[0], color0[1], color0[2], 255);
						break;
					case 1:
						setRGBAPixel(pixPtr, color1[0], color1[1], color1[2], 255);
						break;
					case 2:
						setRGBAPixel(pixPtr, (color0[0] + color1[0]) / 2, (color0[1] + color1[1]) / 2, (color0[2] + color1[2]) / 2, 255);
						break;
					case 3:
						setRGBAPixel(pixPtr, 0, 0, 0, 255);
						break;
					}
				}
			}
		}
	}
}

void DecompressImageDXT1(int width, int height, BYTE *compressedImage, BYTE *image) {
	int blockCountX = width / 4;
	int blockCountY = height / 4;

	for (int j = 0; j < blockCountY; j++) {
		for (int i = 0; i < blockCountX; i++) {
			decompressBlock(i * 4, j * 4, width, compressedImage, image);
			compressedImage += 8;
		}
	}
}