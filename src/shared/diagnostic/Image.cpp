#include "Image.hpp"

#include <cstdio>
#include <iostream>
#include <memory>
#include <ostream>
#include <png.h>
#include <stdexcept>

void DumpImageError(const char* path, FILE* fp = nullptr, png_structp pngPtr = nullptr, png_infop info = nullptr)
{
	std::cout << "Failed to dump image file " << path << std::endl;

	if (fp != nullptr)
	{
		fclose(fp);
	}

	if (pngPtr != nullptr)
	{
		png_destroy_write_struct(&pngPtr, &info);
	}
}

extern uint8_t defaultPalette[1024];

void DumpImage(const char* path, uint8_t* data, uint32_t width, uint32_t height, uint8_t* palette)
{
	if (palette == nullptr)
	{
		palette = defaultPalette;
	}

	FILE *fp = fopen(path, "wb");

	if (!fp)
	{
		DumpImageError(path);
		return;
	}

	png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

	if (!pngPtr)
	{
		DumpImageError(path, fp);
		return;
	}

	png_infop infoPtr = png_create_info_struct(pngPtr);

	if (!infoPtr)
	{
		DumpImageError(path, fp, pngPtr);
		return;
	}

	png_init_io(pngPtr, fp);

	uint32_t colorType = PNG_COLOR_TYPE_PALETTE;

	png_set_IHDR(pngPtr, infoPtr, width, height, 8, colorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	if (height > 640)
	{
		throw std::runtime_error("Too big!");
	}

	uint8_t* rows[640];

	for(int i = 0; i < height; i++)
	{
		rows[i] = data + i * width;
	}

	png_set_rows(pngPtr, infoPtr, rows);

	png_color pngPalette[256];

	for(int i = 0; i < 256; i++)
	{
		memcpy(&pngPalette[i], palette + i * 4, 3);
	}

	png_set_PLTE(pngPtr, infoPtr, pngPalette, 256);

	png_write_png(pngPtr, infoPtr, PNG_TRANSFORM_IDENTITY, nullptr);

	png_write_end(pngPtr, infoPtr);

	png_destroy_write_struct(&pngPtr, nullptr);

	fclose(fp);
}

void DumpImageRGB(const char* path, uint8_t* data, uint32_t width, uint32_t height)
{
	FILE *fp = fopen(path, "wb");

	if (!fp)
	{
		DumpImageError(path);
		return;
	}

	png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

	if (!pngPtr)
	{
		DumpImageError(path, fp);
		return;
	}

	png_infop infoPtr = png_create_info_struct(pngPtr);

	if (!infoPtr)
	{
		DumpImageError(path, fp, pngPtr);
		return;
	}

	png_init_io(pngPtr, fp);

	png_set_IHDR(pngPtr, infoPtr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	if (height > 640)
	{
		throw std::runtime_error("Too big!");
	}

	const int pixelSize = 3;
	uint8_t* rows[640];

	for(int i = 0; i < height; i++)
	{
		rows[i] = data + i * width * pixelSize;
	}

	png_set_rows(pngPtr, infoPtr, rows);

	png_write_png(pngPtr, infoPtr, PNG_TRANSFORM_IDENTITY, nullptr);

	png_write_end(pngPtr, infoPtr);

	png_destroy_write_struct(&pngPtr, nullptr);

	fclose(fp);
}

// automatically generated
uint8_t defaultPalette[1024] = {
	0,      0,      0,      0,      39,     39,     59,     0,      47,     47,     71,     0,      55,     59,     83,     0,
	47,     51,     75,     0,      43,     43,     67,     0,      39,     39,     59,     0,      83,     87,     119,    0,
	75,     79,     111,    0,      71,     75,     103,    0,      79,     83,     115,    0,      87,     95,     131,    0,
	99,     107,    147,    0,      107,    119,    163,    0,      58,     0,      58,     0,      25,     0,      25,     0,
	44,     36,     24,     0,      72,     36,     20,     0,      92,     44,     20,     0,      112,    48,     20,     0,
	104,    60,     36,     0,      124,    64,     24,     0,      120,    76,     44,     0,      168,    8,      8,      0,
	140,    84,     48,     0,      132,    96,     68,     0,      160,    84,     28,     0,      196,    76,     24,     0,
	188,    104,    36,     0,      180,    112,    60,     0,      208,    100,    32,     0,      220,    148,    52,     0,
	224,    148,    84,     0,      236,    196,    84,     0,      52,     68,     40,     0,      64,     108,    60,     0,
	72,     108,    80,     0,      76,     128,    80,     0,      80,     140,    92,     0,      92,     160,    120,    0,
	0,      0,      24,     0,      0,      16,     52,     0,      0,      8,      80,     0,      36,     52,     72,     0,
	48,     64,     84,     0,      20,     52,     124,    0,      52,     76,     108,    0,      64,     88,     116,    0,
	72,     104,    140,    0,      0,      112,    156,    0,      88,     128,    164,    0,      64,     104,    212,    0,
	24,     172,    184,    0,      36,     36,     252,    0,      100,    148,    188,    0,      112,    168,    204,    0,
	140,    192,    216,    0,      148,    220,    244,    0,      172,    220,    232,    0,      172,    252,    252,    0,
	204,    248,    248,    0,      252,    252,    0,      0,      244,    228,    144,    0,      252,    252,    192,    0,
	12,     12,     12,     0,      24,     20,     16,     0,      28,     28,     32,     0,      40,     40,     48,     0,
	56,     48,     36,     0,      56,     60,     68,     0,      76,     64,     48,     0,      76,     76,     76,     0,
	92,     80,     64,     0,      88,     88,     88,     0,      104,    104,    104,    0,      120,    132,    108,    0,
	104,    148,    108,    0,      116,    164,    124,    0,      152,    148,    140,    0,      144,    184,    148,    0,
	152,    196,    168,    0,      176,    176,    176,    0,      172,    204,    176,    0,      196,    192,    188,    0,
	204,    224,    208,    0,      240,    240,    240,    0,      28,     16,     8,      0,      40,     24,     12,     0,
	52,     16,     8,      0,      52,     32,     12,     0,      56,     16,     32,     0,      52,     40,     32,     0,
	68,     52,     8,      0,      72,     48,     24,     0,      96,     0,      0,      0,      84,     40,     32,     0,
	80,     64,     20,     0,      92,     84,     20,     0,      132,    4,      4,      0,      104,    76,     52,     0,
	124,    56,     48,     0,      112,    100,    32,     0,      124,    80,     80,     0,      164,    52,     28,     0,
	148,    108,    0,      0,      152,    92,     64,     0,      140,    128,    52,     0,      152,    116,    84,     0,
	184,    84,     68,     0,      176,    144,    24,     0,      176,    116,    92,     0,      244,    4,      4,      0,
	200,    120,    84,     0,      252,    104,    84,     0,      224,    164,    132,    0,      252,    148,    104,    0,
	252,    204,    44,     0,      16,     252,    24,     0,      12,     0,      32,     0,      28,     28,     44,     0,
	36,     36,     76,     0,      40,     44,     104,    0,      44,     48,     132,    0,      32,     24,     184,    0,
	52,     60,     172,    0,      104,    104,    148,    0,      100,    144,    252,    0,      124,    172,    252,    0,
	0,      228,    252,    0,      156,    144,    64,     0,      168,    148,    84,     0,      188,    164,    92,     0,
	204,    184,    96,     0,      232,    216,    128,    0,      236,    196,    176,    0,      252,    252,    56,     0,
	252,    252,    124,    0,      252,    252,    164,    0,      8,      8,      8,      0,      16,     16,     16,     0,
	24,     24,     24,     0,      40,     40,     40,     0,      52,     52,     52,     0,      76,     60,     56,     0,
	68,     68,     68,     0,      72,     72,     88,     0,      88,     88,     104,    0,      116,    104,    56,     0,
	120,    100,    92,     0,      96,     96,     124,    0,      132,    116,    116,    0,      132,    132,    156,    0,
	172,    140,    124,    0,      172,    152,    148,    0,      144,    144,    184,    0,      184,    184,    232,    0,
	248,    140,    20,     0,      16,     84,     60,     0,      32,     144,    112,    0,      44,     180,    148,    0,
	4,      32,     100,    0,      72,     28,     80,     0,      8,      52,     152,    0,      104,    48,     120,    0,
	136,    64,     156,    0,      12,     72,     204,    0,      188,    184,    52,     0,      220,    220,    60,     0,
	16,     0,      0,      0,      36,     0,      0,      0,      52,     0,      0,      0,      72,     0,      0,      0,
	96,     24,     4,      0,      140,    40,     8,      0,      200,    24,     24,     0,      224,    44,     44,     0,
	232,    32,     32,     0,      232,    80,     20,     0,      252,    32,     32,     0,      232,    120,    36,     0,
	248,    172,    60,     0,      0,      20,     0,      0,      0,      40,     0,      0,      0,      68,     0,      0,
	0,      100,    0,      0,      8,      128,    8,      0,      36,     152,    36,     0,      60,     156,    60,     0,
	88,     176,    88,     0,      104,    184,    104,    0,      128,    196,    128,    0,      148,    212,    148,    0,
	12,     20,     36,     0,      36,     60,     100,    0,      48,     80,     132,    0,      56,     92,     148,    0,
	72,     116,    180,    0,      84,     132,    196,    0,      96,     148,    212,    0,      120,    180,    236,    0,
	20,     16,     8,      0,      24,     20,     12,     0,      40,     48,     12,     0,      16,     16,     24,     0,
	20,     20,     32,     0,      44,     44,     64,     0,      68,     76,     104,    0,      4,      4,      4,      0,
	28,     24,     16,     0,      32,     28,     20,     0,      36,     32,     28,     0,      48,     40,     28,     0,
	64,     56,     44,     0,      84,     72,     52,     0,      104,    92,     76,     0,      144,    124,    100,    0,
	40,     32,     16,     0,      44,     36,     20,     0,      52,     44,     24,     0,      56,     44,     28,     0,
	60,     48,     28,     0,      64,     52,     32,     0,      68,     56,     36,     0,      80,     68,     36,     0,
	88,     76,     40,     0,      100,    88,     44,     0,      12,     16,     4,      0,      20,     24,     4,      0,
	28,     32,     8,      0,      32,     40,     12,     0,      52,     60,     16,     0,      64,     72,     16,     0,
	32,     32,     48,     0,      20,     20,     20,     0,      32,     24,     28,     0,      32,     32,     32,     0,
	40,     32,     24,     0,      40,     36,     36,     0,      48,     44,     44,     0,      60,     48,     56,     0,
	60,     56,     60,     0,      72,     60,     48,     0,      68,     52,     64,     0,      84,     64,     72,     0,
	92,     100,    100,    0,      108,    116,    120,    0,      88,     78,     47,     0,      77,     67,     44,     0,
	71,     59,     43,     0,      75,     63,     47,     0,      83,     67,     51,     0,      67,     75,     103,    0,
	75,     83,     111,    0,      83,     91,     123,    0,      91,     99,     135,    0,      255,    255,    255,    0,
};