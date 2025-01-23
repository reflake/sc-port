#pragma once

#include <cstdint>

extern void DumpImage(const char* path, uint8_t* data, uint32_t width, uint32_t height, uint8_t* palette = nullptr);