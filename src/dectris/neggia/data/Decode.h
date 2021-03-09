// SPDX-License-Identifier: MIT

#ifndef DECODE_H
#define DECODE_H
#include <cstdlib>

#define LZ4_FILTER 32004
#define BSHUF_H5FILTER 32008
#define BSHUF_H5_COMPRESS_LZ4 2

void lz4Decode(const char* inBuffer, char* outBuffer, size_t& outBufferSize);
void bshufUncompressLz4(const char* inBuffer,
                        char* outBuffer,
                        size_t& outBufferSize,
                        size_t elementSize);

#endif  // DECODE_H
