#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctime>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#include "dxt1.h"
#include "types.h"
#include "colors.h"

int main() {
	char inPath[] = "./images/pretty.png";
	char outPath[] = "./output/out.png";
	int width, height, bytesPerPixel;
	long pixelCount;
	BYTE *imageData;

	BYTE *oldImageData = stbi_load(inPath, &width, &height, &bytesPerPixel, 0);
	if (oldImageData == NULL) {
		printf("Invalid image!");
		return  1;
	}
	if (width % 4 != 0 || height % 4 != 0) {
		printf("Invalid dimensions");
		return  1;
	}
	if (width > 32768 || height > 32768) {
		printf("Max image dimensions: 32768x32768");
		return  1;
	}

	pixelCount = width*height;

	// Check if image is RGB and convert to RGBA
	imageData = (BYTE *)malloc(pixelCount * 4);
	if (bytesPerPixel == 3) {
		bytesPerPixel = 4;
		rgb_to_rgba_image(oldImageData, imageData, pixelCount);
	} else if (bytesPerPixel == 4) {
		memcpy(imageData, oldImageData, pixelCount*bytesPerPixel);
	} else {
		printf("Invalid pixel size! %d %d %d", bytesPerPixel, width, height);
		return 1;
	}

	BYTE *compressedImage = (BYTE*)malloc(pixelCount * 4 / 8);
	BYTE* decompressedImage = (BYTE*)malloc(pixelCount * 4);

	time_t compressStart = clock();
	// Compress image DXT1(8:1 compression ratio)
	CompressImageDXT1(imageData, compressedImage, width, height);
	time_t compressEnd = clock();

	// Decompress image DXT1
	DecompressImageDXT1(width, height, compressedImage, decompressedImage);

	time_t decompressEnd = clock();

	printf("compress: %.3fsec\n", double(compressEnd - compressStart) / CLOCKS_PER_SEC);
	printf("decompress: %.3fsec\n", double(decompressEnd - compressEnd) / CLOCKS_PER_SEC);
	printf("total: %.3fsec\n", double(decompressEnd - compressStart) / CLOCKS_PER_SEC);


	//if (stbi_write_jpg(outPath, width, height, bytesPerPixel, imageData, 100)) {
	if (stbi_write_png(outPath, width, height, bytesPerPixel, decompressedImage, width*bytesPerPixel)) {
		printf("WRITE SUCCESS!");
	} else {
		printf("WRITE ERROR!");
	}

	stbi_image_free(imageData);
	stbi_image_free(oldImageData);

	return 0;
}